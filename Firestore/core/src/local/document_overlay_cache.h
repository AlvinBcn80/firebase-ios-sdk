/*
 * Copyright 2022 Google LLC
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *      http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#ifndef FIRESTORE_CORE_SRC_LOCAL_DOCUMENT_OVERLAY_CACHE_H_
#define FIRESTORE_CORE_SRC_LOCAL_DOCUMENT_OVERLAY_CACHE_H_

#include <cstdlib>
#include <string>
#include <unordered_map>

#include "Firestore/core/src/model/document_key.h"
#include "Firestore/core/src/model/mutation.h"
#include "Firestore/core/src/model/mutation/overlay.h"
#include "Firestore/core/src/model/resource_path.h"
#include "absl/types/optional.h"

namespace firebase {
namespace firestore {
namespace local {

/**
 * Provides methods to read and write document overlays.
 *
 * An overlay is a saved `Mutation`, that gives a local view of a document when
 * applied to the remote version of the document.
 *
 * Each overlay stores the largest batch ID that is included in the overlay,
 * which allows us to remove the overlay once all batches leading up to it have
 * been acknowledged.
 */
class DocumentOverlayCache {
 public:
  using OverlayByDocumentKeyMap = std::unordered_map<model::DocumentKey,
                                                     model::mutation::Overlay,
                                                     model::DocumentKeyHash>;
  using MutationByDocumentKeyMap = std::unordered_map<model::DocumentKey,
                                                      model::Mutation,
                                                      model::DocumentKeyHash>;

  virtual ~DocumentOverlayCache() = default;

  /**
   * Gets the saved overlay mutation for the given document key.
   *
   * Returns an empty optional if there is no overlay for that key.
   */
  virtual absl::optional<model::mutation::Overlay> GetOverlay(
      const model::DocumentKey& key) const = 0;

  /**
   * Saves the given document key to mutation map to persistence as overlays.
   *
   * All overlays will have their largest batch id set to `largestBatchId`.
   */
  virtual void SaveOverlays(int largest_batch_id,
                            const MutationByDocumentKeyMap& overlays) = 0;

  /** Removes the overlay whose largest-batch-id equals to the given ID. */
  virtual void RemoveOverlaysForBatchId(int batch_id) = 0;

  /**
   * Returns all saved overlays for the given collection.
   *
   * @param collection The collection path to get the overlays for.
   * @param since_batch_id The minimum batch ID to filter by (exclusive).
   *     Only overlays that contain a change past `sinceBatchId` are returned.
   * @return Mapping of each document key in the collection to its overlay.
   */
  virtual OverlayByDocumentKeyMap GetOverlays(
      const model::ResourcePath& collection, int since_batch_id) const = 0;

  /**
   * Returns `count` overlays with a batch ID higher than `sinceBatchId` for the
   * provided collection group, processed by ascending batch ID.
   *
   * This method always returns all overlays for a batch even if the last batch
   * contains more documents than the remaining limit.
   *
   * @param collection_group The collection group to get the overlays for.
   * @param since_batch_id The minimum batch ID to filter by (exclusive).
   *     Only overlays that contain a change past `sinceBatchId` are returned.
   * @param count The number of overlays to return. Can be exceeded if the last
   *     batch contains more entries.
   * @return Mapping of each document key in the collection group to its
   * overlay.
   */
  virtual OverlayByDocumentKeyMap GetOverlays(
      const std::string& collection_group,
      int since_batch_id,
      std::size_t count) const = 0;
};

}  // namespace local
}  // namespace firestore
}  // namespace firebase

#endif  // FIRESTORE_CORE_SRC_LOCAL_DOCUMENT_OVERLAY_CACHE_H_
