/** \file AnimationControl.cpp */
#include "AnimationControl.h"

AnimationControl::AnimationControl() {
	m_currentTime = 0.0f;
}

AnimationControl::AnimationControl(std::function<void(SimTime, SimTime, Table<String, float>)> c) {
	m_currentTime = 0.0f;
	m_callback = c;
}

void AnimationControl::invoke(SimTime sdt) {
	m_currentTime += sdt;
	m_callback(sdt, m_currentTime, args);
}