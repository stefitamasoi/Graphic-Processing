#include "Camera.hpp"
#include <iostream>

#include "glm/ext.hpp"
#include "glm/glm.hpp"
#include "glm/gtx/transform.hpp"
#include "glm/gtx/string_cast.hpp"

#include <string>
namespace gps
{
    // Camera constructor
    Camera::Camera(glm::vec3 cameraPosition, glm::vec3 cameraTarget, glm::vec3 cameraUp)
    {
        //DONE
        this->cameraPosition = cameraPosition;
        this->cameraTarget = cameraTarget;
        this->cameraUpDirection = cameraUp;

        this->cameraFrontDirection = glm::normalize(cameraTarget - cameraPosition);
        this->cameraRightDirection = glm::normalize(glm::cross(cameraUp, cameraFrontDirection));
    }

    // return the view matrix, using the glm::lookAt() function
    glm::mat4 Camera::getViewMatrix()
    {
        //DONE
        return glm::lookAt(cameraPosition, cameraTarget, this->cameraUpDirection);
    }

    // update the camera internal parameters following a camera move event
    void Camera::move(MOVE_DIRECTION direction, float speed)
    {
        //DONE
        //use a switch statement to determine which direction to move the camera
        glm::vec3 moveDirection;
        switch (direction)
        {
        case MOVE_FORWARD:
            moveDirection = cameraFrontDirection;
            break;
        case MOVE_BACKWARD:
            moveDirection = -cameraFrontDirection;
            break;
        case MOVE_RIGHT:
            moveDirection = -cameraRightDirection;
            break;
        case MOVE_LEFT:
            moveDirection = cameraRightDirection;
            break;
        }
        cameraPosition += moveDirection * speed;
        cameraTarget += moveDirection * speed;
    }

    // update the camera internal parameters following a camera rotate event
    // yaw - camera rotation around the y axis
    // pitch - camera rotation around the x axis
    void Camera::rotate(float pitch, float yaw)
    {
        //DONE
        //update the camera target to rotate the camera
        //use glm::rotate() to update the camera target
        //but constrain the camera so that it cannot look straight up or straight down

        glm::vec3 newCameraTarget = cameraTarget - cameraPosition;
        glm::mat4 rotationMatrix = glm::mat4(1.0f);
        rotationMatrix = glm::rotate(rotationMatrix, glm::radians(yaw), cameraUpDirection);
        newCameraTarget = glm::vec3(rotationMatrix * glm::vec4(newCameraTarget, 1.0f));
        newCameraTarget = glm::normalize(newCameraTarget);

        glm::vec3 right = glm::cross(cameraUpDirection, newCameraTarget);
        rotationMatrix = glm::mat4(1.0f);
        rotationMatrix = glm::rotate(rotationMatrix, glm::radians(pitch), right);
        newCameraTarget = glm::vec3(rotationMatrix * glm::vec4(newCameraTarget, 1.0f));
        newCameraTarget = glm::normalize(newCameraTarget);

        if (glm::abs(glm::dot(newCameraTarget, cameraUpDirection)) < 0.99)
        {
            cameraTarget = cameraPosition + newCameraTarget;
        }

        cameraFrontDirection = glm::normalize(cameraTarget - cameraPosition);
        cameraRightDirection = glm::normalize(glm::cross(cameraUpDirection, cameraFrontDirection));
    }    
    
    void Camera::afisCamera() {
        std::cout << glm::to_string(this->cameraPosition) << std::endl;
        std::cout << glm::to_string(this->cameraTarget) << std::endl;
        std::cout << std::endl;
    }

}