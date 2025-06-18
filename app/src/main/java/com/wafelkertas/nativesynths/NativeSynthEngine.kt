package com.wafelkertas.nativesynths

/**
 * NativeSynthEngine - A Kotlin interface to the native C++ audio synthesizer.
 */
class NativeSynthEngine {

    /**
     * Initializes the native audio engine (OpenSL ES).
     */
    external fun initEngine()

    /**
     * Sets the oscillator frequency in Hz.
     * @param freq Frequency in Hz (e.g. 440.0)
     */
    external fun setFrequency(freq: Double)

    /**
     * Sets the master output volume.
     * @param vol Volume in range [0.0, 1.0]
     */
    external fun setVolume(vol: Double)

    /**
     * Sets the waveform type.
     * @param wave 0: sine, 1: square, 2: saw, 3: triangle
     */
    external fun setWaveform(wave: Int)

    companion object {
        init {
            System.loadLibrary("native-synth") // Ensure the .so is correctly built and loaded
        }
    }
}