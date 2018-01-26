// Copyright (C) 2017 The Android Open Source Project
//
// Licensed under the Apache License, Version 2.0 (the "License");
// you may not use this file except in compliance with the License.
// You may obtain a copy of the License at
//
//      http://www.apache.org/licenses/LICENSE-2.0
//
// Unless required by applicable law or agreed to in writing, software
// distributed under the License is distributed on an "AS IS" BASIS,
// WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
// See the License for the specific language governing permissions and
// limitations under the License.

#pragma once

#include "frameworks/base/cmds/statsd/src/statsd_config.pb.h"
#include "statslog.h"
#include "src/logd/LogEvent.h"
#include "src/StatsLogProcessor.h"

namespace android {
namespace os {
namespace statsd {

// Create AtomMatcher proto to simply match a specific atom type.
AtomMatcher CreateSimpleAtomMatcher(const string& name, int atomId);

// Create AtomMatcher proto for acquiring wakelock.
AtomMatcher CreateAcquireWakelockAtomMatcher();

// Create AtomMatcher proto for releasing wakelock.
AtomMatcher CreateReleaseWakelockAtomMatcher() ;

// Create AtomMatcher proto for screen turned on.
AtomMatcher CreateScreenTurnedOnAtomMatcher();

// Create AtomMatcher proto for screen turned off.
AtomMatcher CreateScreenTurnedOffAtomMatcher();

// Create AtomMatcher proto for app sync turned on.
AtomMatcher CreateSyncStartAtomMatcher();

// Create AtomMatcher proto for app sync turned off.
AtomMatcher CreateSyncEndAtomMatcher();

// Create AtomMatcher proto for app sync moves to background.
AtomMatcher CreateMoveToBackgroundAtomMatcher();

// Create AtomMatcher proto for app sync moves to foreground.
AtomMatcher CreateMoveToForegroundAtomMatcher();

// Create AtomMatcher proto for process crashes
AtomMatcher CreateProcessCrashAtomMatcher() ;

// Create Predicate proto for screen is on.
Predicate CreateScreenIsOnPredicate();

// Create Predicate proto for screen is off.
Predicate CreateScreenIsOffPredicate();

// Create Predicate proto for holding wakelock.
Predicate CreateHoldingWakelockPredicate();

// Create a Predicate proto for app syncing.
Predicate CreateIsSyncingPredicate();

// Create a Predicate proto for app is in background.
Predicate CreateIsInBackgroundPredicate();

// Add a predicate to the predicate combination.
void addPredicateToPredicateCombination(const Predicate& predicate, Predicate* combination);

// Create dimensions from primitive fields.
FieldMatcher CreateDimensions(const int atomId, const std::vector<int>& fields);

// Create dimensions by attribution uid and tag.
FieldMatcher CreateAttributionUidAndTagDimensions(const int atomId,
                                                  const std::vector<Position>& positions);

// Create dimensions by attribution uid only.
FieldMatcher CreateAttributionUidDimensions(const int atomId,
                                            const std::vector<Position>& positions);

// Create log event for screen state changed.
std::unique_ptr<LogEvent> CreateScreenStateChangedEvent(
    const android::view::DisplayStateEnum state, uint64_t timestampNs);

// Create log event for app moving to background.
std::unique_ptr<LogEvent> CreateMoveToBackgroundEvent(const int uid, uint64_t timestampNs);

// Create log event for app moving to foreground.
std::unique_ptr<LogEvent> CreateMoveToForegroundEvent(const int uid, uint64_t timestampNs);

// Create log event when the app sync starts.
std::unique_ptr<LogEvent> CreateSyncStartEvent(
    const int uid, const string& name, uint64_t timestampNs);

// Create log event when the app sync ends.
std::unique_ptr<LogEvent> CreateSyncEndEvent(
    const int uid, const string& name, uint64_t timestampNs);

// Create log event when the app sync ends.
std::unique_ptr<LogEvent> CreateAppCrashEvent(
    const int uid, uint64_t timestampNs);

// Create log event for acquiring wakelock.
std::unique_ptr<LogEvent> CreateAcquireWakelockEvent(
    const std::vector<AttributionNode>& attributions,
    const string& wakelockName, uint64_t timestampNs);

// Create log event for releasing wakelock.
std::unique_ptr<LogEvent> CreateReleaseWakelockEvent(
    const std::vector<AttributionNode>& attributions,
    const string& wakelockName, uint64_t timestampNs);

// Create log event for releasing wakelock.
std::unique_ptr<LogEvent> CreateIsolatedUidChangedEvent(
    int isolatedUid, int hostUid, bool is_create, uint64_t timestampNs);

// Helper function to create an AttributionNode proto.
AttributionNode CreateAttribution(const int& uid, const string& tag);

// Create a statsd log event processor upon the start time in seconds, config and key.
sp<StatsLogProcessor> CreateStatsLogProcessor(const long timeBaseSec, const StatsdConfig& config,
                                              const ConfigKey& key);

// Util function to sort the log events by timestamp.
void sortLogEventsByTimestamp(std::vector<std::unique_ptr<LogEvent>> *events);

int64_t StringToId(const string& str);

void ValidateAttributionUidDimension(const DimensionsValue& value, int atomId, int uid);
void ValidateAttributionUidAndTagDimension(
    const DimensionsValue& value, int atomId, int uid, const std::string& tag);

template <typename T>
void sortMetricDataByDimensionsValue(const T& metricData, T* sortedMetricData) {
    std::map<HashableDimensionKey, int> dimensionIndexMap;
    for (int i = 0; i < metricData.data_size(); ++i) {
        dimensionIndexMap.insert(std::make_pair(metricData.data(i).dimensions_in_what(), i));
    }
    for (const auto& itr : dimensionIndexMap) {
        *sortedMetricData->add_data() = metricData.data(itr.second);
    }
}

}  // namespace statsd
}  // namespace os
}  // namespace android