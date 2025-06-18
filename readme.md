# 🎧 WafelNativeSynths – Migrating to Oboe

**WafelNativeSynths** is a personal learning project that demonstrates how to implement a high-performance real-time synthesizer engine on Android using C++, JNI, and the [Oboe](https://github.com/google/oboe) audio library. This project began with OpenSL ES but has since been migrated to Oboe for improved performance, better API design, and future-proofing.

---

## 🚀 Project Goals

- Migrate from OpenSL ES to **Oboe** for lower latency and modern audio features.
- Learn how to integrate **C++ audio engines** into Android using the **NDK** and **CMake**.
- Use **JNI** to call native C++ audio code from Kotlin UI.
- Generate and control real-time waveforms: **sine**, **square**, **sawtooth**, and **triangle**.
- Add support for **polyphony**, **volume normalization**, and **continuous playback**.
- Understand Oboe's **callback-driven architecture** and **stream configuration**.

---

## 🧠 Why Oboe?

OpenSL ES is deprecated and has inconsistencies across Android devices. Oboe is a modern C++ wrapper that simplifies working with audio streams, unifying **AAudio** (Android 8.1+) and **OpenSL ES** under the hood. Benefits include:

- ✅ Lower latency
- ✅ Better error handling and debug info
- ✅ Cleaner and more maintainable code
- ✅ Easier support for modern Android devices