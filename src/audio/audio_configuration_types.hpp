#ifndef __AUDIO_CONFIGURATION_TYPES_HPP__
#define __AUDIO_CONFIGURATION_TYPES_HPP__

#include <cstdint>
#include <optional>
#include <string>

class AudacityProject;

namespace au::audio {
    /** Represents an optional audio device name; std::nullopt means "no device selected". */
    using AudioDeviceSelection = std::optional<std::string>;

    /** Full snapshot of the audio subsystem configuration. */
    struct AudioConfiguration {
        std::string api;
        AudioDeviceSelection outputDevice;
        AudioDeviceSelection inputDevice;
        int32_t inputChannels = 1;
        double_t bufferLength = 0.0;
        bool automaticLatencyCompensation = false;
        double_t latencyCompensation = 0.0;
        uint64_t defaultSampleRate = 0;
        std::string defaultSampleFormat;
        /** For ASIO: prefer the sample rate exposed by the device itself. */
        bool asioUseDeviceSampleRate = true;
    };

    /** Partial update to AudioConfiguration; only set fields are applied. */
    struct AudioConfigurationChange {
        std::optional<std::string> api;
        std::optional<AudioDeviceSelection> outputDevice;
        std::optional<AudioDeviceSelection> inputDevice;
        std::optional<int32_t> inputChannels;
        std::optional<double_t> bufferLength;
        std::optional<bool> automaticLatencyCompensation;
        std::optional<double_t> latencyCompensation;
        std::optional<uint64_t> defaultSampleRate;
        std::optional<std::string> defaultSampleFormat;
        std::optional<bool> asioUseDeviceSampleRate;
    };

    /** Bit flags identifying individual AudioConfiguration fields. */
    enum class AudioConfigurationField : uint32_t {
        None = 0,
        Api = 1 << 0,
        OutputDevice = 1 << 1,
        InputDevice = 1 << 2,
        InputChannels = 1 << 3,
        BufferLength = 1 << 4,
        AutomaticLatencyCompensation = 1 << 5,
        LatencyCompensation = 1 << 6,
        DefaultSampleRate = 1 << 7,
        DefaultSampleFormat = 1 << 8,
        AsioUseDeviceSampleRate = 1 << 9,
    };

    using AudioConfigurationFields = uint32_t;

    /** Converts an AudioConfigurationField enum value into a bitmask value. */
    constexpr AudioConfigurationFields fieldMask(AudioConfigurationField field) {
        return static_cast<AudioConfigurationFields>(field);
    }

    /** Describes which fields changed between two configurations. */
    struct AudioConfigurationDelta {
        AudioConfigurationFields fields = fieldMask(AudioConfigurationField::None);

        bool contains(AudioConfigurationField field) const {
            return (fields & fieldMask(field)) != 0;
        }

        bool empty() const { return fields == 0; }
    };

    /** Результат применения AudioConfigurationChange. */
    enum class ApplyStatus {
        Applied,
        NoChange,
        InvalidConfiguration,
        InvalidRouting,
        NoUsableAudioApi,
        NoAsioDevice,
        OwnerUnavailable,
        Busy,
        InternalError,
    };

    /** Detailed result of applying a configuration change. */
    struct ApplyResult {
        ApplyStatus status = ApplyStatus::InternalError;
        bool streamRestorationFailed = false;

        bool succeeded() const {
            return status == ApplyStatus::Applied || status == ApplyStatus::NoChange;
        }
    };

    /** Describes a change of audio routing (api / input / output device). */
    struct AudioRoutingChange {
        std::optional<std::string> api;
        std::optional<AudioDeviceSelection> outputDevice;
        std::optional<AudioDeviceSelection> inputDevice;
    };

    /** Logical category of an audio stream. */
    enum class AudioStreamKind {
        Playback,
        Monitoring,
        Recording,
    };

    /** Describes a single audio stream to be opened. */
    struct AudioStreamDescriptor {
        AudioStreamKind kind = AudioStreamKind::Playback;
        /** Invariant: AudioIO owns at most one process-wide stream. */
        AudacityProject *ownerProject = nullptr;
        double sampleRate = 0.0;
    };
}

#endif
