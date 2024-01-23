//  Copyright (c) 2024 Feng Yang
//
//  I am making my contributions/submissions to this project solely in my
//  personal capacity and am not conveying any rights to any intellectual
//  property of any third parties.

#pragma once

#include <Metal/Metal.hpp>

#include <pxr/base/gf/vec3f.h>
#include <pxr/usd/usd/stage.h>
#include <pxr/imaging/hdx/tokens.h>
#include <pxr/imaging/hdx/types.h>
#include <pxr/imaging/hgi/blitCmdsOps.h>
#include <pxr/usdImaging/usdImagingGL/engine.h>
#include <pxr/usd/usdGeom/bboxCache.h>
#import <QWidget>

#include "swapchain.h"
#include "camera.h"
#include "../model/data_model.h"

namespace vox {
class Viewport : public QWidget {
public:
    explicit Viewport(QWidget *parent, DataModel &model);

    [[nodiscard]] QPaintEngine *paintEngine() const override { return nullptr; }

    /// Draw the scene, and blit the result to the view.
    /// Returns false if the engine wasn't initialized.
    void draw();

    void recreateSwapChain(QSize size);

    void resizeEvent(QResizeEvent *event) override;

    pxr::UsdImagingGLRendererSettingsList rendererSettingLists();

    pxr::VtValue rendererSetting(pxr::TfToken const &id);

    void setRendererSetting(pxr::TfToken const &id, pxr::VtValue const &value);

private:
    /// Initializes the Storm engine.
    void initializeEngine();

    /// Updates the animation timing variables.
    double updateTime();

    /// Draws the scene using Hydra.
    pxr::HgiTextureHandle drawWithHydra(double timeCode, CGSize viewSize);

private:
    /// Loads the scene from the provided URL and prepares the camera.
    void setupScene();

    /// Requests the bounding box cache from Hydra.
    pxr::UsdGeomBBoxCache computeBboxCache();

    /// Determine the size of the world so the camera will frame its entire bounding box.
    void calculateWorldCenterAndSize();

    /// Sets a camera up so that it sees the entire scene.
    void setupCamera();

    /// Gets important information about the scene, such as frames per second and if the z-axis points up.
    void getSceneInformation();

private:
    DataModel &_model;

    dispatch_semaphore_t _inFlightSemaphore{};
    pxr::HgiUniquePtr _hgi;
    std::unique_ptr<pxr::UsdImagingGLEngine> _engine;
    std::unique_ptr<Swapchain> _swapchain{};

    double _startTimeInSeconds{};
    double _timeCodesPerSecond{};
    double _startTimeCode{};
    double _endTimeCode{};

    pxr::GfVec3d _worldCenter{};
    double _worldSize{};
    bool _isZUp{};
    std::unique_ptr<Camera> _viewCamera;
};
}// namespace vox