#pragma once
#include <JuceHeader.h>
#include "PlayerAudio.h"

class PlayerGUI : public juce::Component,
                  private juce::Button::Listener,
                  private juce::Slider::Listener,
                  private juce::Timer,
                  private juce::ListBoxModel
{
public:
    PlayerGUI(PlayerAudio& player);
    ~PlayerGUI() override;
    void paint(juce::Graphics&) override;
    void resized() override;
    
    class SessionListener {
    public:
        virtual ~SessionListener() = default;
        virtual void saveSessionRequest() = 0;
        virtual void loadSessionRequest() = 0;
    };
    void setSessionListener(SessionListener* listener) { sessionListener = listener; }

private:
    PlayerAudio& audio;
    SessionListener* sessionListener = nullptr;

    
    juce::TextButton loadButton{"Load"}, playButton{"Play"}, pauseButton{"Pause"},
                     stopButton{"Stop"}, restartButton{"Restart"}, muteButton{"Mute"},
                     loopButton{"Loop"}, backButton{"⏪10s"}, fwdButton{"10s⏩"},
                     aButton{"Set A"}, bButton{"Set B"}, clearABButton{"Clear A-B"},
                     addMarkerButton{"Add Marker"}, fadeInButton{"Fade In"}, fadeOutButton{"Fade Out"},
                     saveButton{"Save Session"}, loadSessionButton{"Load Session"};

    
    juce::Label fileLabel, timeLabel;
    juce::Slider positionSlider, speedSlider, volumeSlider;

    
    juce::ComboBox markersList;
    juce::ListBox playlistBox;
    juce::StringArray playlist;

    bool aSet = false;
    bool bSet = false;

    void buttonClicked(juce::Button*) override;
    void sliderValueChanged(juce::Slider*) override;
    void timerCallback() override;
    void updateUI();

    int getNumRows() override;
    void paintListBoxItem(int, juce::Graphics&, int, int, bool) override;
    void selectedRowsChanged(int) override;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(PlayerGUI)
};