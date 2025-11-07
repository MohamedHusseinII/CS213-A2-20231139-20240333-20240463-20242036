#include "PlayerGUI.h"

juce::String formatTime(double seconds)
{
    int mins = static_cast<int>(seconds) / 60;
    int secs = static_cast<int>(seconds) % 60;
    return juce::String(mins).paddedLeft('0', 2) + ":" + juce::String(secs).paddedLeft('0', 2);
}

PlayerGUI::PlayerGUI(PlayerAudio& p) : audio(p)
{
    for (auto* b : { &loadButton, &playButton, &pauseButton, &stopButton, &restartButton,
                     &muteButton, &loopButton, &backButton, &fwdButton,
                     &aButton, &bButton, &clearABButton, &addMarkerButton,
                     &fadeInButton, &fadeOutButton, &saveButton, &loadSessionButton })
    {
        addAndMakeVisible(*b);
        b->addListener(this);
    }

    addAndMakeVisible(fileLabel);
    addAndMakeVisible(timeLabel);
    addAndMakeVisible(positionSlider);
    addAndMakeVisible(speedSlider);
    addAndMakeVisible(volumeSlider);
    addAndMakeVisible(markersList);
    addAndMakeVisible(playlistBox);

    fileLabel.setText("No file loaded", juce::dontSendNotification);
    timeLabel.setText("00:00 / 00:00", juce::dontSendNotification);

    positionSlider.setRange(0.0, 1.0);
    positionSlider.addListener(this);

    speedSlider.setRange(0.5, 2.0);
    speedSlider.setValue(1.0);
    speedSlider.addListener(this);

    volumeSlider.setRange(0.0, 1.0);
    volumeSlider.setValue(1.0);
    volumeSlider.addListener(this);

    markersList.addItem("Markers:", 0);
    markersList.onChange = [this]()
    {
        int index = markersList.getSelectedId() - 1;
        if (index >= 0)
            audio.jumpToMarker(index);
    };

    playlistBox.setModel(this);
    startTimerHz(30);
}

PlayerGUI::~PlayerGUI()
{
    for (auto* b : { &loadButton, &playButton, &pauseButton, &stopButton, &restartButton,
                     &muteButton, &loopButton, &backButton, &fwdButton,
                     &aButton, &bButton, &clearABButton, &addMarkerButton,
                     &fadeInButton, &fadeOutButton, &saveButton, &loadSessionButton })
        b->removeListener(this);
}

void PlayerGUI::paint(juce::Graphics& g)
{
    g.fillAll(juce::Colours::darkgrey);
    g.setColour(juce::Colours::white);
    g.drawText("Enhanced Audio Player - Final Version", getLocalBounds().removeFromTop(20), juce::Justification::centred);
}

void PlayerGUI::resized()
{
    auto area = getLocalBounds().reduced(6);

    auto topRow = area.removeFromTop(30);
    for (auto* b : { &loadButton, &playButton, &pauseButton, &stopButton, &restartButton })
        b->setBounds(topRow.removeFromLeft(70));

    auto row2 = area.removeFromTop(30);
    for (auto* b : { &backButton, &fwdButton, &muteButton, &loopButton })
        b->setBounds(row2.removeFromLeft(70));

    auto row3 = area.removeFromTop(30);
    for (auto* b : { &aButton, &bButton, &clearABButton, &addMarkerButton })
        b->setBounds(row3.removeFromLeft(80));

    auto row4 = area.removeFromTop(30);
    for (auto* b : { &fadeInButton, &fadeOutButton, &saveButton, &loadSessionButton })
        b->setBounds(row4.removeFromLeft(100));

    fileLabel.setBounds(10, 160, getWidth() - 20, 24);
    timeLabel.setBounds(10, 190, getWidth() - 20, 24);
    positionSlider.setBounds(10, 220, getWidth() - 20, 20);
    speedSlider.setBounds(10, 250, getWidth() - 20, 20);
    volumeSlider.setBounds(10, 280, getWidth() - 20, 20);
    playlistBox.setBounds(10, 310, getWidth() - 20, 120);
    markersList.setBounds(10, 440, getWidth() - 20, 24);
}

void PlayerGUI::buttonClicked(juce::Button* b)
{
    if (b == &loadButton)
    {
        auto chooser = std::make_shared<juce::FileChooser>("Select Audio Files...", juce::File{}, "*.mp3;*.wav;*.aiff");
        chooser->launchAsync(juce::FileBrowserComponent::openMode | juce::FileBrowserComponent::canSelectMultipleItems,
            [this, chooser](const juce::FileChooser& fc)
            {
                auto results = fc.getResults();
                if (results.size() > 0)
                {
                    playlist.clear();
                    for (const auto& file : results)
                        playlist.add(file.getFullPathName());
                    playlistBox.updateContent();

                    juce::File firstFile(playlist[0]);
                    if (audio.loadURL(juce::URL(firstFile)))
                    {
                        fileLabel.setText(firstFile.getFileName(), juce::dontSendNotification);
                        audio.play();
                    }
                }
            });
    }
    else if (b == &playButton) audio.play();
    else if (b == &pauseButton) audio.pause();
    else if (b == &stopButton) audio.stop();
    else if (b == &restartButton) audio.restart();
    else if (b == &muteButton) audio.toggleMute();
    else if (b == &loopButton)
    {
        bool nowLooping = !audio.isLooping();
        audio.setLooping(nowLooping);
        loopButton.setColour(juce::TextButton::buttonColourId, nowLooping ? juce::Colours::orange : juce::Colours::lightgrey);
    }
    else if (b == &backButton) audio.skipSeconds(-10.0);
    else if (b == &fwdButton) audio.skipSeconds(10.0);
    else if (b == &aButton)
    {
        audio.setABPoints(audio.getPosition(), -1);
        aSet = true;
        aButton.setColour(juce::TextButton::buttonColourId, juce::Colours::orange);
    }
    else if (b == &bButton)
    {
        audio.setABPoints(-1, audio.getPosition());
        bSet = true;
        bButton.setColour(juce::TextButton::buttonColourId, juce::Colours::orange);
    }
    else if (b == &clearABButton)
    {
        audio.clearAB();
        aSet = bSet = false;
        aButton.setColour(juce::TextButton::buttonColourId, juce::Colours::lightgrey);
        bButton.setColour(juce::TextButton::buttonColourId, juce::Colours::lightgrey);
    }
    else if (b == &addMarkerButton)
    {
        audio.addMarker(audio.getPosition());
        markersList.clear();
        int id = 1;
        for (double pos : audio.getMarkers())
            markersList.addItem("Marker " + formatTime(pos), id++);
    }
    else if (b == &fadeInButton)
    {
        float gain = audio.getGain();
        if (gain < 1.0f)
            audio.setGain(gain + 0.1f);
    }
    else if (b == &fadeOutButton)
    {
        float gain = audio.getGain();
        if (gain > 0.0f)
            audio.setGain(gain - 0.1f);
    }
    else if (b == &saveButton && sessionListener) sessionListener->saveSessionRequest();
    else if (b == &loadSessionButton && sessionListener) sessionListener->loadSessionRequest();
}

void PlayerGUI::sliderValueChanged(juce::Slider* s)
{
    if (s == &positionSlider)
        audio.setPosition(s->getValue() * audio.getLength());
    else if (s == &speedSlider)
        audio.setSpeed((float)s->getValue());
    else if (s == &volumeSlider)
        audio.setGain((float)s->getValue());
}

void PlayerGUI::timerCallback()
{
    updateUI();

    double pointA = audio.getPointA();
    double pointB = audio.getPointB();

    if (audio.isLooping() && pointA >= 0 && pointB > pointA)
    {
        if (audio.getPosition() >= pointB - 0.05)
        {
            audio.setPosition(pointA);
            if (!audio.isPlaying())
                audio.play();
        }
    }
}

void PlayerGUI::updateUI()
{
    if (audio.getLength() > 0)
    {
        double pos = audio.getPosition();
        double len = audio.getLength();
        positionSlider.setValue(pos / len, juce::dontSendNotification);
        timeLabel.setText(formatTime(pos) + " / " + formatTime(len), juce::dontSendNotification);
    }
}

int PlayerGUI::getNumRows() { return playlist.size(); }

void PlayerGUI::paintListBoxItem(int rowNumber, juce::Graphics& g, int width, int height, bool rowIsSelected)
{
    if (rowNumber < 0 || rowNumber >= playlist.size()) return;
    if (rowIsSelected) g.fillAll(juce::Colours::lightblue);
    g.setColour(juce::Colours::white);
    g.drawText(juce::File(playlist[rowNumber]).getFileName(), 4, 0, width - 4, height, juce::Justification::centredLeft);
}

void PlayerGUI::selectedRowsChanged(int lastRowSelected)
{
    if (lastRowSelected >= 0 && lastRowSelected < playlist.size())
    {
        juce::File file(playlist[lastRowSelected]);
        if (file.existsAsFile())
        {
            audio.loadURL(juce::URL(file));
            fileLabel.setText(file.getFileName(), juce::dontSendNotification);
            audio.play();
        }
    }
}