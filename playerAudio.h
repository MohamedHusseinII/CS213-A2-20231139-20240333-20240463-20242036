#pragma once
#include <JuceHeader.h>

class PlayerAudio : public juce::AudioSource
{
public:
    PlayerAudio();
    ~PlayerAudio() override;

    void prepareToPlay (int samplesPerBlockExpected, double sampleRate) override;
    void releaseResources() override;
    void getNextAudioBlock (const juce::AudioSourceChannelInfo& bufferToFill) override;

    bool loadURL (const juce::URL& audioURL);
    
    void play();
    void pause();
    void stop();
    void restart();
    void toggleMute();

    void setLooping(bool shouldLoop) { looping = shouldLoop; }
    bool isLooping() const { return looping; }

    juce::String getTrackTitle() const; 
    juce::File getCurrentFile() const;
    void setPosition (double seconds);
    double getPosition() const;
    double getLength() const;
    bool isPlaying() const;

    void setSpeed (float ratio);
    void skipSeconds (double seconds);

    void setGain(float g);
    float getGain() const;

    void setABPoints (double a, double b);
    void clearAB();
    double getPointA() const { return pointA; }
    double getPointB() const { return pointB; }

    void addMarker (double sec);
    const std::vector<double>& getMarkers() const;
    void jumpToMarker (int index);

    juce::AudioFormatManager formatManager;

private:
    juce::AudioTransportSource transportSource;
    std::unique_ptr<juce::AudioFormatReaderSource> readerSource;
    std::unique_ptr<juce::ResamplingAudioSource> resampler;

    juce::File loadedFile; 
    juce::String currentTrackTitle = "No Track Loaded";
    bool muted = false;
    float prevGain = 1.0f;
    float speedRatio = 1.0f;
    bool looping = false;
    double pointA = -1.0, pointB = -1.0;
    std::vector<double> markers;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (PlayerAudio)
};