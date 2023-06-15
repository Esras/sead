#ifndef SEAD_PROJECTION_H_
#define SEAD_PROJECTION_H_

#include <gfx/seadCamera.h>
#include <gfx/seadGraphics.h>
#include <math/seadBoundBox.h>
#include <math/seadMatrix.h>
#include <math/seadVector.h>
#include <prim/seadRuntimeTypeInfo.h>
#include "math/seadMathNumbers.h"

namespace sead
{

class Camera;
class Viewport;

template <typename T>
class Ray;

class Projection
{
    SEAD_RTTI_BASE(Projection)

public:
    enum Type
    {
        cType_Perspective = 0,
        cType_Ortho = 1,
        cType_Undefined = 2
    };

    Projection();
    virtual ~Projection() = default;

    void setDirty() { mDirty = true; }
    void setDeviceDirty() { mDeviceDirty = true; }

    void setDevicePosture(Graphics::DevicePosture pose)
    {
        mDevicePosture = pose;
        setDeviceDirty();
    }

    const Matrix44f& getProjectionMatrix() const;
    Matrix44f& getProjectionMatrixMutable();

    const Matrix44f& getDeviceProjectionMatrix() const;

    void cameraPosToScreenPos(Vector3f* dst, const Vector3f& camera_pos) const;
    void screenPosToCameraPos(Vector3f* dst, const Vector3f& screen_pos) const;
    void screenPosToCameraPos(Vector3f* dst, const Vector2f& screen_pos) const;

    void project(Vector2f* dst, const Vector3f& camera_pos, const Viewport& viewport) const;
    void unproject(Vector3f* dst, const Vector3f& screen_pos, const Camera& camera) const;
    void unprojectRay(Ray<Vector3f>* dst, const Vector3f& screen_pos, const Camera& camera) const;

    virtual float getNear() const = 0;
    virtual float getFar() const = 0;
    virtual float getFovy() const = 0;
    virtual float getAspect() const = 0;
    virtual void getOffset(Vector2f* offset) const = 0;

    virtual void updateAttributesForDirectProjection();
    virtual Type getProjectionType() const = 0;
    virtual void doUpdateMatrix(Matrix44f* dst) const = 0;
    virtual void doUpdateDeviceMatrix(Matrix44f* dst, const Matrix44f& src,
                                      Graphics::DevicePosture pose) const;
    virtual void doScreenPosToCameraPosTo(Vector3f* dst, const Vector3f& screen_pos) const;

private:
    void updateMatrixImpl_() const;

    mutable bool mDirty;
    mutable bool mDeviceDirty;
    Matrix44f mMatrix;
    Matrix44f mDeviceMatrix;
    Graphics::DevicePosture mDevicePosture;
    f32 mDeviceZScale;
    f32 mDeviceZOffset;
};
#ifdef cafe
static_assert(sizeof(Projection) == 0x94, "sead::Projection size mismatch");
#endif  // cafe

class PerspectiveProjection : public Projection
{
    SEAD_RTTI_OVERRIDE(PerspectiveProjection, Projection)

public:
    PerspectiveProjection();
    PerspectiveProjection(f32 near, f32 far, f32 fovy_rad, f32 aspect);
    ~PerspectiveProjection() override = default;

    void set(f32 _near, f32 _far, f32 fovy_rad, f32 aspect);

    void setNear(f32 near)
    {
        mNear = near;
        setDirty();
    }

    void setFar(f32 far)
    {
        mFar = far;
        setDirty();
    }

    void setFovx(f32 fovx);

    void setFovy(f32 fovy) { setFovy_(fovy); }

    void setAspect(f32 aspect)
    {
        mAspect = aspect;
        setDirty();
    }

    void setOffset(const Vector2f& offset)
    {
        mOffset = offset;
        setDirty();
    }

    void createDividedProjection(PerspectiveProjection* dst, s32 partno_x, s32 partno_y,
                                 s32 divnum_x, s32 divnum_y) const;

    f32 getNear() const override { return mNear; }
    f32 getFar() const override { return mFar; }
    f32 getFovy() const override { return mAngle; }
    f32 getAspect() const override { return mAspect; }
    void getOffset(Vector2f* offset) const override { offset->set(mOffset); }

    f32 getTop() const;
    f32 getBottom() const;
    f32 getLeft() const;
    f32 getRight() const;

    void setTBLR(f32 top, f32 bottom, f32 left, f32 right);

    Type getProjectionType() const override { return Projection::cType_Perspective; }
    void doUpdateMatrix(Matrix44f* dst) const override;
    void doScreenPosToCameraPosTo(Vector3f* dst, const Vector3f& screen_pos) const override;

private:
    void setFovy_(f32 fovy);

    f32 calcNearClipHeight_() const { return mNear * 2 * mFovyTan; }
    f32 calcNearClipWidth_() const { return calcNearClipHeight_() * mAspect; }

    f32 mNear = 1.0f;
    f32 mFar = 10000.0f;
    f32 mAngle;
    f32 mFovySin;
    f32 mFovyCos;
    f32 mFovyTan;
    f32 mAspect = 1.333333f;
    Vector2f mOffset = Vector2f::zero;
};
#ifdef cafe
static_assert(sizeof(PerspectiveProjection) == 0xB8, "sead::PerspectiveProjection size mismatch");
#endif  // cafe

class OrthoProjection : public Projection
{
    SEAD_RTTI_OVERRIDE(OrthoProjection, Projection)

public:
    OrthoProjection();
    OrthoProjection(f32 _near, f32 _far, f32 top, f32 bottom, f32 left, f32 right);
    OrthoProjection(f32 _near, f32 _far, const BoundBox2f& box);
    OrthoProjection(f32 _near, f32 _far, const Viewport& vp);
    ~OrthoProjection() override = default;

    void setNear(f32 near);
    void setFar(f32 far);
    void setTop(f32 top);
    void setBottom(f32 bottom);
    void setLeft(f32 left);
    void setRight(f32 right);
    void setTBLR(f32 top, f32 bottom, f32 left, f32 right);
    void setByViewport(const Viewport& vp);
    void setBoundBox(const BoundBox2f& box);

    void createDividedProjection(OrthoProjection* dst, s32 partno_x, s32 partno_y, s32 divnum_x,
                                 s32 divnum_y) const;

    f32 getNear() const override { return mNear; }
    f32 getFar() const override { return mFar; }
    f32 getFovy() const override { return 0; }
    f32 getTop() const { return mTop; }
    f32 getBottom() const { return mBottom; }
    f32 getLeft() const { return mLeft; }
    f32 getRight() const { return mRight; }
    f32 getAspect() const override;
    void getOffset(Vector2f* offset) const override;

    Type getProjectionType() const override { return Projection::cType_Ortho; }
    void doUpdateMatrix(Matrix44f* dst) const override;
    void doScreenPosToCameraPosTo(Vector3f* dst, const Vector3f& screen_pos) const override;

private:
    f32 mNear;
    f32 mFar;
    f32 mTop;
    f32 mBottom;
    f32 mLeft;
    f32 mRight;
    Vector2f mOffset;
};
#ifdef cafe
static_assert(sizeof(OrthoProjection) == 0xAC, "sead::OrthoProjection size mismatch");
#endif  // cafe

class FrustumProjection : public Projection
{
    SEAD_RTTI_OVERRIDE(FrustumProjection, Projection)

public:
    FrustumProjection() = default;
    FrustumProjection(f32 _near, f32 _far, f32 top, f32 bottom, f32 left, f32 right);
    FrustumProjection(f32 _near, f32 _far, const BoundBox2f& box);
    ~FrustumProjection() override = default;

    Type getProjectionType() const override { return Projection::cType_Perspective; }
    void doUpdateMatrix(Matrix44f* dst) const override;
    void doScreenPosToCameraPosTo(Vector3f* dst, const Vector3f& screen_pos) const override;

    void setNear(f32 near);
    void setFar(f32 far);
    void setTop(f32 top);
    void setBottom(f32 bottom);
    void setLeft(f32 left);
    void setRight(f32 right);

    void setTBLR(f32 top, f32 bottom, f32 left, f32 right);

    void setBoundBox(const BoundBox2f& box);

    void createDividedProjection(FrustumProjection* dst, s32 partno_x, s32 partno_y, s32 divnum_x,
                                 s32 divnum_y) const;

    f32 getNear() const override { return mNear; }
    f32 getFar() const override { return mFar; }
    f32 getTop() const { return mTop; }
    f32 getBottom() const { return mBottom; }
    f32 getLeft() const { return mLeft; }
    f32 getRight() const { return mRight; }

    f32 getFovy() const override;
    f32 getAspect() const override;
    void getOffset(Vector2f* dst) const override;
    f32 getOffsetX() const;
    f32 getOffsetY() const;

    void setFovyAspectOffset(f32 fovy, f32 aspect, const Vector2f& offset);

private:
    f32 mNear;
    f32 mFar;
    f32 mTop;
    f32 mBottom;
    f32 mLeft;
    f32 mRight;
};

class DirectProjection : public Projection
{
    SEAD_RTTI_OVERRIDE(DirectProjection, Projection)

public:
    DirectProjection();
    DirectProjection(const Matrix44f* mtx, Graphics::DevicePosture posture);
    ~DirectProjection() override = default;

    void updateAttributesForDirectProjection() override;
    Type getProjectionType() const override { return cType_Undefined; }
    void doUpdateMatrix(Matrix44f* dst) const override;

    void setDirectProjectionMatrix(const Matrix44f* mtx, Graphics::DevicePosture posture);

    f32 getNear() const override { return mNear; }
    f32 getFar() const override { return mFar; }
    f32 getFovy() const override { return mFovy; }

    f32 getAspect() const override { return mAspect; }
    void getOffset(Vector2f* offset) const override { *offset = mOffset; }

    void doScreenPosToCameraPosTo(Vector3f* dst, const Vector3f& screen_pos) const override;

private:
    Matrix44f mDirectMatrix = Matrix44f::ident;
    f32 mNear = 0.0;
    f32 mFar = 0.0;
    f32 mFovy = 0.0;
    f32 mAspect = 0.0;
    Vector2f mOffset = Vector2f::zero;
    bool someBool = true;
};
#ifdef cafe
static_assert(sizeof(FrustumProjection) == 0xAC, "sead::FrustumProjection size mismatch");
#endif  // cafe

}  // namespace sead

#endif  // SEAD_PROJECTION_H_
