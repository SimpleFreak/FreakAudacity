# File audio_configuration_hpp

``` cpp

/**Represents an optional audio device name; std::nullopt means "no device selected". */
using AudioDeviceSelection = std::optional<std::string>;

/** Full snapshot of the audio subsystem configuration. */
struct AudioConfiguration{
    /** For ASIO: prefer the sample rate exposed by the device itself. */
    bool asioUseDeviceSampleRate = true;
};

/** Partial update to AudioConfiguration; only set fields are applied. */
struct AudioConfigurationChange{};

/** Bit flags identifying individual AudioConfiguration fields. */
enum class AudioConfigurationField : uint32_t;

/** Converts an AudioConfigurationField enum value into a bitmask value. */
constexpr AudioConfigurationFields fieldMask(AudioConfigurationField field) {
    return static_cast<AudioConfigurationFields>(field);
}

/** Describes which fields changed between two configurations. */
struct AudioConfigurationDelta{};

/** Результат применения AudioConfigurationChange. */
enum class ApplyStatus{};

/** Detailed result of applying a configuration change. */
struct ApplyResult{};

/** Describes a change of audio routing (api / input / output device). */
struct AudioRoutingChange{};

/** Logical category of an audio stream. */
enum class AudioStreamKind{};

/** Describes a single audio stream to be opened. */
struct AudioStreamDescriptor{
    /** Invariant: AudioIO owns at most one process-wide stream. */
    AudacityProject *ownerProject = nullptr;
};

```
