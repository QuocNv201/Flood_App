package com.quocnv.floodapp.viewmodel

import androidx.lifecycle.LiveData
import androidx.lifecycle.MutableLiveData
import androidx.lifecycle.ViewModel
import androidx.lifecycle.viewModelScope
import com.quocnv.floodapp.model.SensorData
import com.quocnv.floodapp.repository.FirebaseRepository
import kotlinx.coroutines.flow.catch
import kotlinx.coroutines.launch

/**
 * ViewModel for [com.quocnv.floodapp.MainActivity].
 *
 * Exposes:
 * - [sensorData]   – latest sensor snapshot from Firebase (LiveData)
 * - [errorMessage] – non-null when a Firebase operation fails
 * - [isSending]    – true while a control command is in-flight
 *
 * All Firebase operations run on [viewModelScope] and are automatically
 * cancelled when the ViewModel is cleared.
 */
class SensorViewModel(
    private val repository: FirebaseRepository = FirebaseRepository()
) : ViewModel() {

    private val _sensorData    = MutableLiveData<SensorData>()
    val sensorData: LiveData<SensorData> = _sensorData

    private val _errorMessage  = MutableLiveData<String?>()
    val errorMessage: LiveData<String?> = _errorMessage

    private val _isSending     = MutableLiveData(false)
    val isSending: LiveData<Boolean> = _isSending

    init {
        startListening()
    }

    // -------------------------------------------------------------------------
    // Sensor data
    // -------------------------------------------------------------------------

    private fun startListening() {
        viewModelScope.launch {
            repository.sensorDataFlow()
                .catch { e -> _errorMessage.postValue("Data stream error: ${e.message}") }
                .collect { data -> _sensorData.postValue(data) }
        }
    }

    // -------------------------------------------------------------------------
    // Control commands
    // -------------------------------------------------------------------------

    /**
     * Sends a servo angle command to the ESP32 via Firebase.
     * Validates the angle before writing.
     */
    fun sendServoCommand(angle: Int) {
        if (angle !in 0..180) {
            _errorMessage.value = "Invalid servo angle: $angle (must be 0–180)"
            return
        }
        _isSending.value = true
        viewModelScope.launch {
            runCatching { repository.sendServoCommand(angle) }
                .onSuccess { _isSending.postValue(false) }
                .onFailure { e ->
                    _isSending.postValue(false)
                    _errorMessage.postValue("Servo command failed: ${e.message}")
                }
        }
    }

    /**
     * Sends an alert-reset command to the ESP32 via Firebase.
     */
    fun sendResetAlert() {
        _isSending.value = true
        viewModelScope.launch {
            runCatching { repository.sendResetAlert() }
                .onSuccess { _isSending.postValue(false) }
                .onFailure { e ->
                    _isSending.postValue(false)
                    _errorMessage.postValue("Reset alert failed: ${e.message}")
                }
        }
    }

    /** Clears a previously shown error so the UI doesn't re-show it. */
    fun clearError() {
        _errorMessage.value = null
    }
}
