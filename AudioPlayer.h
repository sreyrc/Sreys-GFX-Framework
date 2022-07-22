#pragma once

#include <irrklang/irrKlang.h>
#include <map>
#include <string>
#include <fstream>

#include "SoundData.h"

class AudioPlayer {

public:
	AudioPlayer() : mSoundEngine(irrklang::createIrrKlangDevice(irrklang::ESOD_WIN_MM)), mSoundData(new SoundData()), mLastPaused(0) {};
	~AudioPlayer() { mSoundEngine->drop(); }

	void Init() {
		std::ifstream input;
		input.open("../resources/music/Tracklist.txt");

		if (!input.is_open()) {
			std::cout << "Ahh shit" << '\n';
		}

		while (input) {
			std::string songName;
			std::getline(input, songName, '\n');
			mSongs[songName] = "../resources/music/" + songName + ".mp3";
		}

		mSongs.erase("");

		mCurrentSongFilePath = mSongs["Jhalak_Dikhlaja"];

		std::cout << mSoundEngine->setMixedDataOutputReceiver(mSoundData) << '\n';
	}

	std::unordered_map<std::string, std::string> GetSongs() {
		return mSongs;
	};

	void SetCurrentSong(std::string song_name) {
		mSoundEngine->stopAllSounds();
		mCurrentSongFilePath = mSongs[song_name];
		mCurrentSong = mSoundEngine->play2D(mCurrentSongFilePath.c_str(), false, false, true);
	}

	void Play() {
		if (mCurrentSong) {
			if (mCurrentSong->getIsPaused()) {
				mCurrentSong = mSoundEngine->play2D(mCurrentSongFilePath.c_str(), false, false, true);
				mCurrentSong->setPlayPosition(mLastPaused);
			}
		}
	}

	void Pause() {
		mLastPaused = mCurrentSong->getPlayPosition();
		auto soundEffect = mCurrentSong->getSoundEffectControl();
		mSoundEngine->setAllSoundsPaused();
	}

	short GetData() {
		return mSoundData->GetData();
	}

private:
	irrklang::ISoundEngine* mSoundEngine;
	irrklang::ISound* mCurrentSong = nullptr;
	irrklang::ik_u32 mLastPaused;
	std::unordered_map<std::string, std::string> mSongs;
	std::string mCurrentSongFilePath;


	SoundData* mSoundData;
};