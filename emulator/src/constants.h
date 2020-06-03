#pragma once

const int   k_mcuClockFreqHz   = 32768;
const int   k_mcuROMSize       = 29 * 64;
const int   k_mcuRAMSize       = 5  * 13;
const int   k_mcuLcdHCount     = 2;
const int   k_mcuLcdOCount     = 9;
const int   k_mcuLcdSCount     = 4;
const float k_lcdFrameTime     = float(512.f / k_mcuClockFreqHz);
const float k_lcdLatencyCoef0  = 32.f;
const float k_lcdLatencyCoef1  = 6.4f;
const int   k_audioFrequencyHz = 44100;
const int   k_audioSamplesNumb = 512;
