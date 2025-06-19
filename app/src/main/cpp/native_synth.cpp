#include <oboe/Oboe.h>
#include <cmath>
#include <mutex>
#include <jni.h>
#include <vector>

class SynthCallback : public oboe::AudioStreamCallback {
public:
    double phase = 0.0;
    double frequency = 440.0;
    double volume = 0.5;
    int waveform = 0; // 0: sine, 1: square, 2: saw, 3: triangle
    std::mutex mutex;
    std::vector<float> lastFrame;

    void setFrequency(double freq) {
        std::lock_guard<std::mutex> lock(mutex);
        frequency = freq;
    }

    void setVolume(double vol) {
        std::lock_guard<std::mutex> lock(mutex);
        volume = std::max(0.0, std::min(1.0, vol));
    }

    void setWaveform(int wave) {
        std::lock_guard<std::mutex> lock(mutex);
        waveform = wave;
    }

    oboe::DataCallbackResult onAudioReady(oboe::AudioStream *stream,
                                          void *audioData,
                                          int32_t numFrames) override {
        float *outputBuffer = static_cast<float *>(audioData);
        double localPhase, localFreq, localVol;
        int localWave;

        {
            std::lock_guard<std::mutex> lock(mutex);
            localFreq = frequency;
            localVol = volume;
            localWave = waveform;
            localPhase = phase;
            lastFrame.assign(outputBuffer, outputBuffer + numFrames);
        }

        double sampleRate = stream->getSampleRate();
        for (int i = 0; i < numFrames; ++i) {
            double value = 0.0;
            double t = localPhase;
            switch (localWave) {
                case 0: // Sine
                    value = sin(2.0 * M_PI * localFreq * t);
                    break;
                case 1: // Square
                    value = sin(2.0 * M_PI * localFreq * t) >= 0 ? 1.0 : -1.0;
                    break;
                case 2: // Sawtooth
                    value = 2.0 * (t * localFreq - floor(t * localFreq + 0.5));
                    break;
                case 3: // Triangle
                    value = 2.0 * fabs(2.0 * (t * localFreq - floor(t * localFreq + 0.5))) - 1.0;
                    break;
            }

            value *= localVol;
            outputBuffer[i] = static_cast<float>(value);
            localPhase += 1.0 / sampleRate;
        }

        {
            std::lock_guard<std::mutex> lock(mutex);
            phase = fmod(localPhase, 1.0); // Wrap phase for long sessions
        }

        return oboe::DataCallbackResult::Continue;
    }
};

static SynthCallback synthCallback;
static oboe::AudioStream *stream = nullptr;

extern "C" {

JNIEXPORT void JNICALL
Java_com_wafelkertas_nativesynths_NativeSynthEngine_initEngine(JNIEnv *, jobject) {
    oboe::AudioStreamBuilder builder;
    builder.setPerformanceMode(oboe::PerformanceMode::LowLatency)
            ->setSharingMode(oboe::SharingMode::Exclusive)
            ->setFormat(oboe::AudioFormat::Float)
            ->setChannelCount(oboe::ChannelCount::Mono)
            ->setCallback(&synthCallback)
            ->openStream(&stream);


}

JNIEXPORT void JNICALL
Java_com_wafelkertas_nativesynths_NativeSynthEngine_setFrequency(JNIEnv *, jobject, jdouble freq) {
    synthCallback.setFrequency(freq);
}

JNIEXPORT void JNICALL
Java_com_wafelkertas_nativesynths_NativeSynthEngine_setVolume(JNIEnv *, jobject, jdouble vol) {
    synthCallback.setVolume(vol);
}

JNIEXPORT void JNICALL
Java_com_wafelkertas_nativesynths_NativeSynthEngine_setWaveform(JNIEnv *, jobject, jint wave) {
    synthCallback.setWaveform(wave);
}

JNIEXPORT void JNICALL
Java_com_wafelkertas_nativesynths_NativeSynthEngine_startEngine(JNIEnv *, jobject) {
    if (stream != nullptr) stream->start();
}

JNIEXPORT void JNICALL
Java_com_wafelkertas_nativesynths_NativeSynthEngine_stopEngine(JNIEnv *, jobject) {
    if (stream) stream->stop();
}

JNIEXPORT jfloatArray JNICALL
Java_com_wafelkertas_nativesynths_NativeSynthEngine_getWaveform(JNIEnv *env, jobject) {
    std::lock_guard<std::mutex> lock(synthCallback.mutex);
    jfloatArray result = env->NewFloatArray(synthCallback.lastFrame.size());
    if (result != nullptr) {
        env->SetFloatArrayRegion(result, 0, synthCallback.lastFrame.size(), synthCallback.lastFrame.data());
    }
    return result;
}

}