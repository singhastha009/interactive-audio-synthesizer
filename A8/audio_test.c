#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <pthread.h>
#include <termios.h>
#include <unistd.h>
#include <AudioToolbox/AudioToolbox.h>
#include <CoreAudio/CoreAudio.h>

#define SAMPLE_RATE 96000.0

// Global variables for real-time control
float frequency = 440.0; // Default frequency (A4)
float volume = 0.5;      // Default volume
int waveform = 1;        // 1 = Sine, 2 = Square, 3 = Sawtooth

// Get user key input without Enter key press
char getKeyPress() {
    struct termios oldt, newt;
    char ch;
    tcgetattr(STDIN_FILENO, &oldt);
    newt = oldt;
    newt.c_lflag &= ~(ICANON | ECHO);
    tcsetattr(STDIN_FILENO, TCSANOW, &newt);
    ch = getchar();
    tcsetattr(STDIN_FILENO, TCSANOW, &oldt);
    return ch;
}

// Background thread for listening to key input
void* listenInput(void* arg) {
    printf("\nControls:\n");
    printf("  w: Increase Frequency  |  s: Decrease Frequency\n");
    printf("  +: Increase Volume  |  -: Decrease Volume\n");
    printf("  1: Sine Wave  |  2: Square Wave  |  3: Sawtooth Wave\n");
    printf("  q: Quit\n");

    while (1) {
        char ch = getKeyPress();

        if (ch == 'w') frequency += 10.0;
        if (ch == 's') frequency -= 10.0;
        if (ch == '+') volume = fmin(1.0, volume + 0.1);
        if (ch == '-') volume = fmax(0.0, volume - 0.1);
        if (ch == '1') waveform = 1;
        if (ch == '2') waveform = 2;
        if (ch == '3') waveform = 3;
        if (ch == 'q') break;

        printf("\rFrequency: %.2f Hz | Volume: %.2f | Waveform: %s    ",
               frequency, volume, (waveform == 1) ? "Sine" : (waveform == 2) ? "Square" : "Sawtooth");
        fflush(stdout);
    }
    exit(0); // Stop program when 'q' is pressed
}

// Function to print device properties
void PrintDeviceProperties() {
    AudioDeviceID deviceID;
    UInt32 size = sizeof(deviceID);
    AudioObjectPropertyAddress propertyAddress = {
        kAudioHardwarePropertyDefaultOutputDevice,
        kAudioObjectPropertyScopeGlobal,
        kAudioObjectPropertyElementMaster
    };

    // Get Default Device ID
    AudioObjectGetPropertyData(kAudioObjectSystemObject, &propertyAddress, 0, NULL, &size, &deviceID);
    printf("🎧 Default Device ID: %u\n", deviceID);

    // Get Sample Rate
    AudioStreamBasicDescription format;
    size = sizeof(format);
    propertyAddress.mSelector = kAudioDevicePropertyStreamFormat;
    propertyAddress.mScope = kAudioObjectPropertyScopeOutput;
    AudioObjectGetPropertyData(deviceID, &propertyAddress, 0, NULL, &size, &format);
    printf("🔊 Sample Rate: %.2f Hz\n", format.mSampleRate);

    // Get Volume Levels
    Float32 leftVolume, rightVolume;
    size = sizeof(leftVolume);
    
    propertyAddress.mSelector = kAudioHardwareServiceDeviceProperty_VirtualMasterVolume;
    if (AudioObjectGetPropertyData(deviceID, &propertyAddress, 0, NULL, &size, &leftVolume) == noErr) {
        printf("📢 Default Volume: Left = %.2f\n", leftVolume);
    } else {
        printf("⚠️ Could not retrieve master volume.\n");
    }

    // Check if it's a stereo device
    propertyAddress.mScope = kAudioDevicePropertyScopeOutput;
    propertyAddress.mSelector = kAudioDevicePropertyVolumeScalar;
    propertyAddress.mElement = 1; // Left channel
    if (AudioObjectGetPropertyData(deviceID, &propertyAddress, 0, NULL, &size, &leftVolume) == noErr) {
        propertyAddress.mElement = 2; // Right channel
        if (AudioObjectGetPropertyData(deviceID, &propertyAddress, 0, NULL, &size, &rightVolume) == noErr) {
            printf("📢 Default Volume: Left = %.2f, Right = %.2f\n", leftVolume, rightVolume);
        }
    }
}

// Audio rendering callback
OSStatus AudioCallback(void *inRefCon, 
                       AudioUnitRenderActionFlags *ioActionFlags,
                       const AudioTimeStamp *inTimeStamp,
                       UInt32 inBusNumber,
                       UInt32 inNumberFrames,
                       AudioBufferList *ioData) {
    static float phase = 0.0;
    float phaseIncrement = 2.0 * M_PI * frequency / SAMPLE_RATE;

    for (UInt32 frame = 0; frame < inNumberFrames; frame++) {
        float sampleValue = 0.0;

        if (waveform == 1) { // Sine wave
            sampleValue = sin(phase);
        } else if (waveform == 2) { // Square wave
            sampleValue = (sin(phase) >= 0) ? 1.0 : -1.0;
        } else if (waveform == 3) { // Sawtooth wave
            sampleValue = (2.0 / M_PI) * (phase - M_PI);
        }

        sampleValue *= volume; // Apply volume
        phase += phaseIncrement;
        if (phase > 2.0 * M_PI) phase -= 2.0 * M_PI;

        for (UInt32 channel = 0; channel < ioData->mNumberBuffers; channel++) {
            float *buffer = (float *)ioData->mBuffers[channel].mData;
            buffer[frame] = sampleValue;
        }
    }

    return noErr;
}

// Main function
int main() {
    printf("🎵 Initializing Audio...\n");
    PrintDeviceProperties();

    // Setup audio unit
    AudioComponentDescription desc = { kAudioUnitType_Output, kAudioUnitSubType_DefaultOutput, kAudioUnitManufacturer_Apple, 0, 0 };
    AudioComponent comp = AudioComponentFindNext(NULL, &desc);
    AudioUnit audioUnit;
    AudioComponentInstanceNew(comp, &audioUnit);

    AURenderCallbackStruct callback = { .inputProc = AudioCallback, .inputProcRefCon = NULL };
    AudioUnitSetProperty(audioUnit, kAudioUnitProperty_SetRenderCallback, kAudioUnitScope_Global, 0, &callback, sizeof(callback));
    AudioUnitInitialize(audioUnit);
    AudioOutputUnitStart(audioUnit);

    // Start input listener thread
    pthread_t inputThread;
    pthread_create(&inputThread, NULL, listenInput, NULL);
    pthread_join(inputThread, NULL);

    // Cleanup
    AudioOutputUnitStop(audioUnit);
    AudioComponentInstanceDispose(audioUnit);
    return 0;
}
