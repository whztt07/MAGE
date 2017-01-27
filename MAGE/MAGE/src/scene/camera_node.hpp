#pragma once

//-----------------------------------------------------------------------------
// Engine Includes
//-----------------------------------------------------------------------------
#pragma region

#include "scene\scene_node.hpp"
#include "camera\camera.hpp"

#pragma endregion

//-----------------------------------------------------------------------------
// Engine Declarations and Definitions
//-----------------------------------------------------------------------------
namespace mage {

	/**
	 A class of camera nodes.
	 */
	class CameraNode : public SceneNode {

	public:

		/**
		 Constructs a camera node with given camera and transform.

		 @pre			@a camera may not point to @c nullptr.
		 @param[in]		camera
						A pointer to the camera.
		 @param[in]		transform
						A reference to the transform.
		 */
		CameraNode(Camera *camera, const Transform &transform = Transform())
			: SceneNode(transform), m_camera(camera) {}
		
		/**
		 Constructs a camera node from the given camera node.

		 @param[in]		camera_node
						The camera node.
		 */
		CameraNode(const CameraNode &camera_node) 
			: SceneNode(camera_node), m_camera(camera_node.m_camera->Clone()) {}

		/**
		 Destructs this camera node.
		 */
		virtual ~CameraNode() {
			SAFE_DELETE(m_camera);
		}

		/**
		 Clones this camera node (non-deep clone).

		 @return		A pointer to a non-deep clone of this camera node.
		 */
		virtual SceneNode *Clone() const {
			return new CameraNode(*this);
		}

		/**
		 Returns the camera of this camera node.

		 @return		A pointer to the camera of this camera node.
		 */
		Camera *GetCamera() const {
			return m_camera;
		}

		/**
		 Accepts the given visitor.

		 @param[in]		visitor
						A reference to the visitor.
		 */
		virtual void Accept(SceneNodeVisitor &visitor) override {
			visitor.VisitCameraNode(*this);
		}
		
		/**
		 Accepts the given visitor.

		 @param[in]		visitor
						A reference to the visitor.
		 */
		virtual void Accept(SceneNodeVisitor &visitor) const override {
			visitor.VisitCameraNode(*this);
		}

	protected:

		/**
		 A pointer to the camera of this camera node.
		 */
		Camera *m_camera;
	};
}
