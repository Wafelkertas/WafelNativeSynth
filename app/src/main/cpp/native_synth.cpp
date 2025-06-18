#include <jni.h>
#include <math.h>
#include <SLES/OpenSLES.h>
#include <SLES/OpenSLES_Android.h>
#include <vector>
#include <mutex>
#include <algorithm>

static SLObjectItf engineObject = nullptr;
static SLEngineItf engineEngine;
static SLObjectItf outputMixObject = nullptr;
static SLObjectItf playerObject = nullptr;
static SLPlayItf playerPlay;
static SLAndroidSimpleBufferQueueItf bufferQueue;

const int sampleRate = 44100;
const int bufferFrames = 512;
short buffer[bufferFrames];

std::mutex audioMutex;
double phase = 0.0;
double frequency = 440.0;
double volume = 0.5;
int waveform = 0; // 0: sine, 1: square, 2: saw, 3: triangle

void generateSamples(short *output, int numSamples) {
    std::lock_guard<std::mutex> lock(audioMutex);
    double phaseIncrement = frequency / sampleRate;

    for (int i = 0; i < numSamples; ++i) {
        double sample = 0.0;

        switch (waveform) {
            case 0: // Sine
                sample = sin(2.0 * M_PI * phase);
                break;
            case 1: // Square
                sample = sin(2.0 * M_PI * phase) >= 0 ? 1.0 : -1.0;
                break;
            case 2: // Sawtooth
                sample = 2.0 * (phase - floor(phase + 0.5));
                break;
            case 3: // Triangle
                sample = 2.0 * fabs(2.0 * (phase - floor(phase + 0.5))) - 1.0;
                break;
            default:
                sample = 0.0;
        }

        sample *= volume;
        sample = std::max(-1.0, std::min(1.0, sample));
        output[i] = static_cast<short>(sample * 32767);

        phase += phaseIncrement;
        if (phase >= 1.0) phase -= 1.0;
    }
}

void bufferCallback(SLAndroidSimpleBufferQueueItf bq, void *context) {
    generateSamples(buffer, bufferFrames);
    (*bq)->Enqueue(bq, buffer, bufferFrames * sizeof(short));
}

extern "C"
JNIEXPORT void JNICALL
Java_com_wafelkertas_nativesynths_NativeSynthEngine_initEngine(JNIEnv *, jobject) {
    SLresult result;

    // Create engine
    result = slCreateEngine(&engineObject, 0, nullptr, 0, nullptr, nullptr);
    if (result != SL_RESULT_SUCCESS) return;

    result = (*engineObject)->Realize(engineObject, SL_BOOLEAN_FALSE);
    if (result != SL_RESULT_SUCCESS) return;

    result = (*engineObject)->GetInterface(engineObject, SL_IID_ENGINE, &engineEngine);
    if (result != SL_RESULT_SUCCESS) return;

    // Create output mix
    result = (*engineEngine)->CreateOutputMix(engineEngine, &outputMixObject, 0, nullptr, nullptr);
    if (result != SL_RESULT_SUCCESS) return;

    result = (*outputMixObject)->Realize(outputMixObject, SL_BOOLEAN_FALSE);
    if (result != SL_RESULT_SUCCESS) return;

    // Configure audio source
    SLDataLocator_AndroidSimpleBufferQueue loc_bufq = {SL_DATALOCATOR_ANDROIDSIMPLEBUFFERQUEUE, 2};
    SLDataFormat_PCM format_pcm = {
            SL_DATAFORMAT_PCM,
            1,
            SL_SAMPLINGRATE_44_1,
            SL_PCMSAMPLEFORMAT_FIXED_16,
            SL_PCMSAMPLEFORMAT_FIXED_16,
            SL_SPEAKER_FRONT_CENTER,
            SL_BYTEORDER_LITTLEENDIAN
    };
    SLDataSource audioSrc = {&loc_bufq, &format_pcm};

    // Configure audio sink
    SLDataLocator_OutputMix loc_outmix = {SL_DATALOCATOR_OUTPUTMIX, outputMixObject};
    SLDataSink audioSnk = {&loc_outmix, nullptr};

    // Create audio player
    const SLInterfaceID ids[] = {SL_IID_BUFFERQUEUE};
    const SLboolean req[] = {SL_BOOLEAN_TRUE};
    result = (*engineEngine)->CreateAudioPlayer(engineEngine, &playerObject, &audioSrc, &audioSnk, 1, ids, req);
    if (result != SL_RESULT_SUCCESS) return;

    result = (*playerObject)->Realize(playerObject, SL_BOOLEAN_FALSE);
    if (result != SL_RESULT_SUCCESS) return;

    result = (*playerObject)->GetInterface(playerObject, SL_IID_PLAY, &playerPlay);
    if (result != SL_RESULT_SUCCESS) return;

    result = (*playerObject)->GetInterface(playerObject, SL_IID_BUFFERQUEUE, &bufferQueue);
    if (result != SL_RESULT_SUCCESS) return;

    result = (*bufferQueue)->RegisterCallback(bufferQueue, bufferCallback, nullptr);
    if (result != SL_RESULT_SUCCESS) return;

    // Start playback
    (*playerPlay)->SetPlayState(playerPlay, SL_PLAYSTATE_PLAYING);
    bufferCallback(bufferQueue, nullptr);
}

extern "C"
JNIEXPORT void JNICALL
Java_com_wafelkertas_nativesynths_NativeSynthEngine_setFrequency(JNIEnv *, jobject, jdouble freq) {
    std::lock_guard<std::mutex> lock(audioMutex);
    frequency = freq;
}

extern "C"
JNIEXPORT void JNICALL
Java_com_wafelkertas_nativesynths_NativeSynthEngine_setVolume(JNIEnv *, jobject, jdouble vol) {
    std::lock_guard<std::mutex> lock(audioMutex);
    volume = std::max(0.0, std::min(1.0, vol));
}

extern "C"
JNIEXPORT void JNICALL
Java_com_wafelkertas_nativesynths_NativeSynthEngine_setWaveform(JNIEnv *, jobject, jint wave) {
    std::lock_guard<std::mutex> lock(audioMutex);
    waveform = wave % 4;  // clamp 0-3
}