/*
 * Copyright (C) 2017 The Android Open Source Project
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

#ifndef DURATION_TRACKER_H
#define DURATION_TRACKER_H

#include "anomaly/AnomalyTracker.h"
#include "condition/ConditionWizard.h"
#include "config/ConfigKey.h"
#include "stats_util.h"

namespace android {
namespace os {
namespace statsd {

enum DurationState {
    kStopped = 0,  // The event is stopped.
    kStarted = 1,  // The event is on going.
    kPaused = 2,   // The event is started, but condition is false, clock is paused. When condition
                   // turns to true, kPaused will become kStarted.
};

// Hold duration information for one atom level duration in current on-going bucket.
struct DurationInfo {
    DurationState state;

    // the number of starts seen.
    int32_t startCount;

    // most recent start time.
    int64_t lastStartTime;
    // existing duration in current bucket.
    int64_t lastDuration;
    // TODO: Optimize the way we track sliced condition in duration metrics.
    // cache the HashableDimensionKeys we need to query the condition for this duration event.
    ConditionKey conditionKeys;

    DurationInfo() : state(kStopped), startCount(0), lastStartTime(0), lastDuration(0){};
};

struct DurationBucket {
    uint64_t mBucketStartNs;
    uint64_t mBucketEndNs;
    uint64_t mDuration;
    uint64_t mBucketNum;
};

class DurationTracker {
public:
    DurationTracker(const ConfigKey& key, const string& name, const HashableDimensionKey& eventKey,
                    sp<ConditionWizard> wizard, int conditionIndex, bool nesting,
                    uint64_t currentBucketStartNs, uint64_t bucketSizeNs,
                    const std::vector<sp<AnomalyTracker>>& anomalyTrackers)
        : mConfigKey(key),
          mName(name),
          mEventKey(eventKey),
          mWizard(wizard),
          mConditionTrackerIndex(conditionIndex),
          mBucketSizeNs(bucketSizeNs),
          mNested(nesting),
          mCurrentBucketStartTimeNs(currentBucketStartNs),
          mDuration(0),
          mCurrentBucketNum(0),
          mAnomalyTrackers(anomalyTrackers){};

    virtual ~DurationTracker(){};

    virtual void noteStart(const HashableDimensionKey& key, bool condition,
                           const uint64_t eventTime, const ConditionKey& conditionKey) = 0;
    virtual void noteStop(const HashableDimensionKey& key, const uint64_t eventTime,
                          const bool stopAll) = 0;
    virtual void noteStopAll(const uint64_t eventTime) = 0;

    virtual void onSlicedConditionMayChange(const uint64_t timestamp) = 0;
    virtual void onConditionChanged(bool condition, const uint64_t timestamp) = 0;

    // Flush stale buckets if needed, and return true if the tracker has no on-going duration
    // events, so that the owner can safely remove the tracker.
    virtual bool flushIfNeeded(
            uint64_t timestampNs,
            std::unordered_map<HashableDimensionKey, std::vector<DurationBucket>>* output) = 0;

    // Predict the anomaly timestamp given the current status.
    virtual int64_t predictAnomalyTimestampNs(const AnomalyTracker& anomalyTracker,
                                              const uint64_t currentTimestamp) const = 0;

protected:
    // Starts the anomaly alarm.
    void startAnomalyAlarm(const uint64_t eventTime) {
        for (auto& anomalyTracker : mAnomalyTrackers) {
            if (anomalyTracker != nullptr) {
                anomalyTracker->startAlarm(mEventKey,
                                           predictAnomalyTimestampNs(*anomalyTracker, eventTime));
            }
        }
    }

    // Stops the anomaly alarm.
    void stopAnomalyAlarm() {
        for (auto& anomalyTracker : mAnomalyTrackers) {
            if (anomalyTracker != nullptr) {
                anomalyTracker->stopAlarm(mEventKey);
            }
        }
    }

    void addPastBucketToAnomalyTrackers(const int64_t& bucketValue, const int64_t& bucketNum) {
        for (auto& anomalyTracker : mAnomalyTrackers) {
            if (anomalyTracker != nullptr) {
                anomalyTracker->addPastBucket(mEventKey, bucketValue, bucketNum);
            }
        }
    }

    void detectAndDeclareAnomaly(const uint64_t& timestamp, const int64_t& currBucketNum,
                                 const int64_t& currentBucketValue) {
        for (auto& anomalyTracker : mAnomalyTrackers) {
            if (anomalyTracker != nullptr) {
                anomalyTracker->detectAndDeclareAnomaly(timestamp, currBucketNum, mEventKey,
                                                        currentBucketValue);
            }
        }
    }

    void declareAnomalyIfAlarmExpired(const uint64_t& timestamp) {
        for (auto& anomalyTracker : mAnomalyTrackers) {
            if (anomalyTracker != nullptr) {
                anomalyTracker->declareAnomalyIfAlarmExpired(mEventKey, timestamp);
            }
        }
    }
    // A reference to the DurationMetricProducer's config key.
    const ConfigKey& mConfigKey;

    const std::string mName;

    HashableDimensionKey mEventKey;

    sp<ConditionWizard> mWizard;

    const int mConditionTrackerIndex;

    const int64_t mBucketSizeNs;

    const bool mNested;

    uint64_t mCurrentBucketStartTimeNs;

    int64_t mDuration;  // current recorded duration result

    uint64_t mCurrentBucketNum;

    std::vector<sp<AnomalyTracker>> mAnomalyTrackers;

    FRIEND_TEST(OringDurationTrackerTest, TestPredictAnomalyTimestamp);
    FRIEND_TEST(OringDurationTrackerTest, TestAnomalyDetection);
};

}  // namespace statsd
}  // namespace os
}  // namespace android

#endif  // DURATION_TRACKER_H
