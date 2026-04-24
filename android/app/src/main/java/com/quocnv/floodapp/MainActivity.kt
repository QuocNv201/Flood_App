package com.quocnv.floodapp

import android.os.Bundle
import android.view.View
import android.widget.SeekBar
import androidx.activity.viewModels
import androidx.appcompat.app.AppCompatActivity
import androidx.core.content.ContextCompat
import com.google.android.material.snackbar.Snackbar
import com.quocnv.floodapp.databinding.ActivityMainBinding
import com.quocnv.floodapp.model.SensorData
import com.quocnv.floodapp.viewmodel.SensorViewModel
import java.text.SimpleDateFormat
import java.util.Date
import java.util.Locale

/**
 * Main (and only) screen of the Flood App Android client.
 *
 * Displays real-time sensor data received from the ESP32 via Firebase
 * Realtime Database and provides controls to:
 *  - Set the servo (flood barrier) to a specific angle.
 *  - Manually reset an active flood alert.
 */
class MainActivity : AppCompatActivity() {

    private lateinit var binding: ActivityMainBinding
    private val viewModel: SensorViewModel by viewModels()

    private val dateFormat = SimpleDateFormat("HH:mm:ss", Locale.getDefault())

    override fun onCreate(savedInstanceState: Bundle?) {
        super.onCreate(savedInstanceState)
        binding = ActivityMainBinding.inflate(layoutInflater)
        setContentView(binding.root)

        setupServoSeekBar()
        setupButtons()
        observeViewModel()
    }

    // -------------------------------------------------------------------------
    // UI setup
    // -------------------------------------------------------------------------

    private fun setupServoSeekBar() {
        binding.seekBarServo.max = 180
        binding.seekBarServo.progress = 0
        updateServoAngleLabel(0)

        binding.seekBarServo.setOnSeekBarChangeListener(object : SeekBar.OnSeekBarChangeListener {
            override fun onProgressChanged(seekBar: SeekBar, progress: Int, fromUser: Boolean) {
                updateServoAngleLabel(progress)
            }
            override fun onStartTrackingTouch(seekBar: SeekBar) {}
            override fun onStopTrackingTouch(seekBar: SeekBar) {}
        })
    }

    private fun setupButtons() {
        binding.btnSetServo.setOnClickListener {
            val angle = binding.seekBarServo.progress
            viewModel.sendServoCommand(angle)
        }

        binding.btnResetAlert.setOnClickListener {
            viewModel.sendResetAlert()
        }
    }

    // -------------------------------------------------------------------------
    // ViewModel observers
    // -------------------------------------------------------------------------

    private fun observeViewModel() {
        viewModel.sensorData.observe(this) { data ->
            updateSensorUI(data)
        }

        viewModel.errorMessage.observe(this) { msg ->
            if (!msg.isNullOrBlank()) {
                showError(msg)
                viewModel.clearError()
            }
        }

        viewModel.isSending.observe(this) { sending ->
            binding.btnSetServo.isEnabled  = !sending
            binding.btnResetAlert.isEnabled = !sending
            binding.progressSending.visibility = if (sending) View.VISIBLE else View.GONE
        }
    }

    // -------------------------------------------------------------------------
    // UI update helpers
    // -------------------------------------------------------------------------

    private fun updateSensorUI(data: SensorData) {
        // ── Distance ──────────────────────────────────────────────────────────
        val distText = if (data.distanceCm > 0)
            getString(R.string.distance_format, data.distanceCm)
        else
            getString(R.string.distance_out_of_range)
        binding.tvDistance.text = distText

        // Progress bar: 0–400 cm mapped to 0–100%
        binding.progressDistance.progress = (data.distanceCm.coerceIn(0.0, 400.0) / 4.0).toInt()

        // ── Light level ───────────────────────────────────────────────────────
        binding.tvLightLevel.text = getString(R.string.light_format, data.lightLevel)
        // LDR ADC max = 4095
        binding.progressLight.progress = (data.lightLevel * 100 / 4095).coerceIn(0, 100)
        binding.tvLightDesc.text = if (data.lightLevel >= 750)
            getString(R.string.light_desc_dark)
        else
            getString(R.string.light_desc_bright)

        // ── Alert status ──────────────────────────────────────────────────────
        val alertColor = if (data.alert)
            ContextCompat.getColor(this, R.color.alert_active)
        else
            ContextCompat.getColor(this, R.color.alert_inactive)

        binding.tvAlertStatus.text = if (data.alert)
            getString(R.string.alert_active)
        else
            getString(R.string.alert_inactive)
        binding.tvAlertStatus.setTextColor(alertColor)
        binding.cardAlert.strokeColor = alertColor

        binding.btnResetAlert.isEnabled = data.alert &&
                (viewModel.isSending.value == false)

        // ── Servo status ──────────────────────────────────────────────────────
        binding.tvServoAngle.text  = getString(R.string.servo_angle_format, data.servoAngle)
        binding.tvServoStatus.text = getString(
            R.string.servo_status_format,
            data.barrierStatus
        )
        binding.progressServoAngle.progress = data.servoAngle

        // ── Last updated ──────────────────────────────────────────────────────
        if (data.lastUpdated > 0) {
            val time = dateFormat.format(Date(data.lastUpdated))
            binding.tvLastUpdated.text = getString(R.string.last_updated_format, time)
        }
    }

    private fun updateServoAngleLabel(angle: Int) {
        binding.tvServoTarget.text = getString(R.string.servo_target_format, angle)
    }

    private fun showError(message: String) {
        Snackbar.make(binding.root, message, Snackbar.LENGTH_LONG).show()
    }
}
