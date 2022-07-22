#pragma once

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>

enum class CameraMovement {
	FORWARD,
	BACKWARD,
	LEFT,
	RIGHT
};

const float YAW = -90.0f;
const float PITCH = 0.0f;
const float SPEED = 2.5f;
const float SENSITIVITY = 0.1f;
const float ZOOM = 45.0f;

class Camera
{
public:
	Camera(glm::vec3 position);
	void Update();
	glm::mat4 GetViewMatrix();
	glm::vec3 GetPosition();
	glm::vec3 GetFront();

	float mZoom;
	glm::vec3 mPosition;
	float mPitch, mYaw;

private:
	glm::vec3 mFront, mRight, mUp;
	glm::vec3 mWorldUp;

	float mMovementSpeed, mMouseSensitivity;
};

