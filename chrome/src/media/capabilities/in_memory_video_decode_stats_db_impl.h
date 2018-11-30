// Copyright 2018 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef MEDIA_CAPABILITIES_IN_MEMORY_VIDEO_DECODE_STATS_DB_IMPL_H_
#define MEDIA_CAPABILITIES_IN_MEMORY_VIDEO_DECODE_STATS_DB_IMPL_H_

#include <map>
#include <memory>

#include "base/files/file_path.h"
#include "base/memory/weak_ptr.h"
#include "components/leveldb_proto/proto_database.h"
#include "media/base/media_export.h"
#include "media/base/video_codecs.h"
#include "media/capabilities/video_decode_stats_db.h"
#include "ui/gfx/geometry/size.h"

namespace media {

class VideoDecodeStatsDBProvider;

// The in-memory database disappears with profile shutdown to preserve the
// privacy of off-the-record (OTR) browsing profiles (Guest and Incognito). It
// also allows the MediaCapabilities API to behave the same both on and
// off-the-record which prevents sites from detecting when users are OTR modes.
// VideoDecodeStatsDBProvider gives incognito profiles a hook to read the stats
// of the of the originating profile. Guest profiles are conceptually a blank
// slate and will not have a "seed" DB.
class MEDIA_EXPORT InMemoryVideoDecodeStatsDBFactory
    : public VideoDecodeStatsDBFactory {
 public:
  // |seed_db_provider| provides access to a seed (read-only) DB instance.
  // Callers must ensure the |seed_db_provider| outlives this factory and any
  // databases it creates via CreateDB(). |seed_db_provider| may be null when no
  // seed DB is available.
  explicit InMemoryVideoDecodeStatsDBFactory(
      VideoDecodeStatsDBProvider* seed_db_provider);
  ~InMemoryVideoDecodeStatsDBFactory() override;

  // DB is not thread-safe and is bound to the sequence used at construction.
  std::unique_ptr<VideoDecodeStatsDB> CreateDB() override;

 private:
  // Provided at construction. Callers must ensure that object outlives this
  // class.
  VideoDecodeStatsDBProvider* seed_db_provider_;

  DISALLOW_COPY_AND_ASSIGN(InMemoryVideoDecodeStatsDBFactory);
};

class MEDIA_EXPORT InMemoryVideoDecodeStatsDBImpl : public VideoDecodeStatsDB {
 public:
  // Constructs the database. NOTE: must call Initialize() before using.
  // |db| injects the level_db database instance for storing capabilities info.
  // |dir| specifies where to store LevelDB files to disk. LevelDB generates a
  // handful of files, so its recommended to provide a dedicated directory to
  // keep them isolated.
  explicit InMemoryVideoDecodeStatsDBImpl(
      VideoDecodeStatsDBProvider* seed_db_provider);
  ~InMemoryVideoDecodeStatsDBImpl() override;

  // Implement VideoDecodeStatsDB.
  void Initialize(InitializeCB init_cb) override;
  void AppendDecodeStats(const VideoDescKey& key,
                         const DecodeStatsEntry& entry,
                         AppendDecodeStatsCB append_done_cb) override;
  void GetDecodeStats(const VideoDescKey& key,
                      GetDecodeStatsCB get_stats_cb) override;
  void DestroyStats(base::OnceClosure destroy_done_cb) override;

 private:
  // Called when the |seed_db_provider_| returns an initialized seed DB. Will
  // run |init_cb|, marking the completion of Initialize().
  void OnGotSeedDB(base::OnceCallback<void(bool)> init_cb,
                   VideoDecodeStatsDB* seed_db);

  // Passed as the callback for |OnGotDecodeStats| by |AppendDecodeStats| to
  // update the database once we've read the existing stats entry.
  void CompleteAppendWithSeedData(const VideoDescKey& key,
                                  const DecodeStatsEntry& entry,
                                  AppendDecodeStatsCB append_done_cb,
                                  bool read_success,
                                  std::unique_ptr<DecodeStatsEntry> seed_entry);

  // Called when GetDecodeStats() operation was performed. |get_stats_cb|
  // will be run with |success| and a |DecodeStatsEntry| created from
  // |stats_proto| or nullptr if no entry was found for the requested key.
  void OnGotSeedEntry(const VideoDescKey& key,
                      GetDecodeStatsCB get_stats_cb,
                      bool success,
                      std::unique_ptr<DecodeStatsEntry> seed_entry);

  // Indicates whether initialization is completed.
  bool db_init_ = false;

  // Lazily provides |seed_db_| from original profile. Owned by original profile
  // and may be null.
  VideoDecodeStatsDBProvider* seed_db_provider_ = nullptr;

  // On-disk DB owned by the base profile for the off-the-record session. For
  // incognito sessions, this will contain the original profile's stats. For
  // guest sessions, this will be null (no notion of base profile). See
  // |in_memory_db_|.
  VideoDecodeStatsDB* seed_db_ = nullptr;

  // In-memory DB, mapping VideoDescKey strings -> DecodeStatsEntries. This is
  // the primary storage (read and write) for this class. The |seed_db_| is
  // read-only, and  will only be queried when the |in_memory_db_| lacks an
  // entry for a given key.
  std::map<std::string, DecodeStatsEntry> in_memory_db_;

  // Ensures all access to class members come on the same sequence. API calls
  // and callbacks should occur on the same sequence used during construction.
  // LevelDB operations happen on a separate task runner, but all LevelDB
  // callbacks to this happen on the checked sequence.
  SEQUENCE_CHECKER(sequence_checker_);

  base::WeakPtrFactory<InMemoryVideoDecodeStatsDBImpl> weak_ptr_factory_;

  DISALLOW_COPY_AND_ASSIGN(InMemoryVideoDecodeStatsDBImpl);
};

}  // namespace media

#endif  // MEDIA_CAPABILITIES_IN_MEMORY_VIDEO_DECODE_STATS_DB_IMPL_H_