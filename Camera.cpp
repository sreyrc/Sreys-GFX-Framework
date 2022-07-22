#include "Camera.h"

Camera::Camera(glm::vec3 position)
	: mPosition(position), mUp(glm::vec3(0.0f, 1.0f, 0.0f)), mPitch(PITCH), mYaw(YAW),
	mFront(glm::vec3(0.0f, 0.0f, -1.0f)), mMovementSpeed(SPEED), mMouseSensitivity(SENSITIVITY), mZoom(ZOOM) {
	mWorldUp = mUp;
	Update();
}

void Camera::Update()
{
	glm::vec3 front;
	front.x = cos(glm::radians(mYaw)) * cos(glm::radians(mPitch));
	front.y = sin(glm::radians(mPitch));
	front.z = sin(glm::radians(mYaw)) * cos(glm::radians(mPitch));

	mFront = glm::normalize(front);
	mRight = glm::normalize(glm::cross(mFront, mWorldUp)); 
	mUp = glm::normalize(glm::cross(mRight, mFront));
}

glm::mat4 Camera::GetViewMatrix() {
	return glm::lookAt(mPosition, mPosition + mFront, mUp);
}

glm::vec3 Camera::GetPosition() {
	return mPosition;
}

glm::vec3 Camera::GetFront()
{
	return mFront;
}

