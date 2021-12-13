#pragma once

class ICamera {

    /**
     * Updates the cameras state and returns whether the camera state has changed since last the invocation of update.
     */
    virtual bool update() = 0;

};