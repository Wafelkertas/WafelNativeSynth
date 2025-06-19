package com.wafelkertas.nativesynths

import android.annotation.SuppressLint
import android.content.Context
import android.graphics.Canvas
import android.graphics.Color
import android.graphics.Paint
import android.os.Bundle
import android.util.AttributeSet
import android.view.View
import android.widget.AdapterView
import android.widget.ArrayAdapter
import android.widget.SeekBar
import androidx.appcompat.app.AppCompatActivity
import androidx.lifecycle.lifecycleScope
import com.wafelkertas.nativesynths.databinding.ActivityMainBinding
import kotlinx.coroutines.Dispatchers
import kotlinx.coroutines.delay
import kotlinx.coroutines.launch
import kotlinx.coroutines.withContext

class MainActivity : AppCompatActivity(), AdapterView.OnItemSelectedListener {

    private val synth = NativeSynthEngine()
    private lateinit var binding: ActivityMainBinding

    override fun onCreate(savedInstanceState: Bundle?) {
        super.onCreate(savedInstanceState)
        binding = ActivityMainBinding.inflate(layoutInflater)
        setContentView(binding.root)

        synth.initEngine()
        initUI()
    }

    private fun initUI() {
        binding.frequencySeekBar.max = 2000
        binding.frequencySeekBar.progress = 440
        binding.frequencySeekBar.setOnSeekBarChangeListener(object : SeekBar.OnSeekBarChangeListener {
            override fun onProgressChanged(seekBar: SeekBar?, progress: Int, fromUser: Boolean) {
                val freq = progress.toDouble().coerceAtLeast(20.0)
                synth.setFrequency(freq)
                binding.frequencyLabel.text = "Frequency: %.1f Hz".format(freq)
            }

            override fun onStartTrackingTouch(seekBar: SeekBar?) {}
            override fun onStopTrackingTouch(seekBar: SeekBar?) {}
        })

        binding.volumeSeekBar.max = 100
        binding.volumeSeekBar.progress = 50
        binding.volumeSeekBar.setOnSeekBarChangeListener(object : SeekBar.OnSeekBarChangeListener {
            override fun onProgressChanged(seekBar: SeekBar?, progress: Int, fromUser: Boolean) {
                synth.setVolume(progress / 100.0)
                binding.volumeLabel.text = "Volume: $progress%"
            }

            override fun onStartTrackingTouch(seekBar: SeekBar?) {}
            override fun onStopTrackingTouch(seekBar: SeekBar?) {}
        })


        val spinner = binding.waveformSpinner
        spinner.onItemSelectedListener = this
        ArrayAdapter.createFromResource(
                this,
                R.array.waveforms,
                android.R.layout.simple_spinner_item
        ).also { adapter ->
            // Specify the layout to use when the list of choices appears.
            adapter.setDropDownViewResource(android.R.layout.simple_spinner_dropdown_item)
            // Apply the adapter to the spinner.
            spinner.adapter = adapter
        }

        binding.playToggle.setOnCheckedChangeListener { _, isChecked ->
            if (isChecked) {
                synth.startEngine()
            } else {
                synth.stopEngine()
            }
        }
        val waveformView = binding.waveformView
        lifecycleScope.launch {
            while (true) {
                val samples = withContext(Dispatchers.Default) {
                    synth.getWaveform()
                }
                waveformView.update(samples)
                delay(50L) // ~20 FPS
            }
        }
    }

    override fun onItemSelected(parent: AdapterView<*>?, view: View?, position: Int, id: Long) {
        synth.setWaveform(position)

    }

    override fun onNothingSelected(parent: AdapterView<*>?) {
    }
}

class WaveformView @JvmOverloads constructor(
    context: Context, attrs: AttributeSet? = null
) : View(context, attrs) {

    private val paint = Paint().apply {
        color = Color.GREEN
        strokeWidth = 2f
        style = Paint.Style.STROKE
        isAntiAlias = true
    }

    private var samples: FloatArray = FloatArray(0)

    fun update(newSamples: FloatArray) {
        samples = newSamples
        invalidate()
    }

    override fun onDraw(canvas: Canvas) {
        super.onDraw(canvas)
        if (samples.isEmpty()) return

        val widthPerSample = width.toFloat() / samples.size
        val centerY = height / 2f

        for (i in 0 until samples.size - 1) {
            val x1 = i * widthPerSample
            val y1 = centerY - samples[i] * centerY
            val x2 = (i + 1) * widthPerSample
            val y2 = centerY - samples[i + 1] * centerY
            canvas.drawLine(x1, y1, x2, y2, paint)
        }
    }
}