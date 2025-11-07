#include "PlayerAudio.h"

PlayerAudio::PlayerAudio()
{
    formatManager.registerBasicFormats();
    resampler = std::make_unique<juce::ResamplingAudioSource>(&transportSource, false, 2);
}

PlayerAudio::~PlayerAudio()
{
    transportSource.stop();
    transportSource.setSource(nullptr);
    readerSource.reset();
}

void PlayerAudio::prepareToPlay(int samplesPerBlockExpected, double sampleRate)
{
    resampler->prepareToPlay(samplesPerBlockExpected, sampleRate);
}

void PlayerAudio::releaseResources()
{
    resampler->releaseResources();
}

void PlayerAudio::getNextAudioBlock(const juce::AudioSourceChannelInfo& bufferToFill)
{
    resampler->getNextAudioBlock(bufferToFill);

    if (looping)
    {
        double pos = transportSource.getCurrentPosition();
        double len = transportSource.getLengthInSeconds();

        if (pointA >= 0 && pointB > pointA)
        {
            if (pos >= pointB)
                setPosition(pointA);
        }
        else if (pos >= len)
        {
            setPosition(0.0);
            play();
        }
    }
}

bool PlayerAudio::loadURL(const juce::URL& audioURL)
{
    transportSource.stop();
    transportSource.setSource(nullptr);
    readerSource.reset();

    auto stream = audioURL.createInputStream(false);
    if (stream == nullptr) return false;

    clearAB();
    markers.clear();

    auto* reader = formatManager.createReaderFor(std::move(stream));
    if (!reader) return false;

    loadedFile = audioURL.getLocalFile();
    currentTrackTitle = reader->metadataValues.getValue("Title", loadedFile.getFileNameWithoutExtension());
    readerSource = std::make_unique<juce::AudioFormatReaderSource>(reader, true);
    transportSource.setSource(readerSource.get(), 0, nullptr, reader->sampleRate);
    resampler->setResamplingRatio(speedRatio);
    transportSource.setGain(prevGain);
    muted = false;
    return true;
}

juce::String PlayerAudio::getTrackTitle() const { return currentTrackTitle; }
juce::File PlayerAudio::getCurrentFile() const { return loadedFile; }
double PlayerAudio::getLength() const { return transportSource.getLengthInSeconds(); }
double PlayerAudio::getPosition() const { return transportSource.getCurrentPosition(); }
bool PlayerAudio::isPlaying() const { return transportSource.isPlaying(); }

void PlayerAudio::play()
{
    if (!transportSource.isPlaying())
        transportSource.start();
}

void PlayerAudio::pause() { transportSource.stop(); }
void PlayerAudio::stop() { transportSource.stop(); transportSource.setPosition(0.0); }
void PlayerAudio::restart() { transportSource.setPosition(0.0); transportSource.start(); }

void PlayerAudio::toggleMute()
{
    if (!muted)
    {
        prevGain = transportSource.getGain();
        transportSource.setGain(0.0f);
        muted = true;
    }
    else
    {
        transportSource.setGain(prevGain);
        muted = false;
    }
}

void PlayerAudio::skipSeconds(double seconds)
{
    double pos = transportSource.getCurrentPosition() + seconds;
    pos = juce::jlimit(0.0, transportSource.getLengthInSeconds(), pos);
    transportSource.setPosition(pos);
}

void PlayerAudio::setSpeed(float ratio)
{
    if (ratio <= 0.0f) return;
    speedRatio = ratio;
    if (resampler) resampler->setResamplingRatio(speedRatio);
}

void PlayerAudio::setPosition(double seconds)
{
    transportSource.setPosition(seconds);
}

void PlayerAudio::setGain(float g)
{
    transportSource.setGain(juce::jlimit(0.0f, 1.0f, g));
}

float PlayerAudio::getGain() const
{
    return transportSource.getGain();
}

void PlayerAudio::setABPoints(double a, double b)
{
    if (a >= 0 && b < 0)
        pointA = a;
    else if (b >= 0 && a < 0)
        pointB = b;
    else if (a >= 0 && b >= 0)
    {
        pointA = juce::jmin(a, b);
        pointB = juce::jmax(a, b);
    }
}

void PlayerAudio::clearAB()
{
    pointA = pointB = -1.0;
}

void PlayerAudio::addMarker(double sec)
{
    if (std::find(markers.begin(), markers.end(), sec) == markers.end())
        markers.push_back(sec);
}

const std::vector<double>& PlayerAudio::getMarkers() const
{
    return markers;
}

void PlayerAudio::jumpToMarker(int index)
{
    if (index >= 0 && index < (int)markers.size())
        setPosition(markers[index]);
}