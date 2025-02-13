#include "include/camera.h"
#include "include/render.h"
namespace Physics2D
{
	void Camera::render(QPainter* painter)
	{
		if (m_visible)
		{
			assert(painter != nullptr && m_world != nullptr);

			real inv_dt = 1.0 / m_deltaTime;

			real scale = m_targetMeterToPixel - m_meterToPixel;
			if (std::fabs(scale) < 0.1 || m_meterToPixel < 1.0)
				m_meterToPixel = m_targetMeterToPixel;
			else
				m_meterToPixel -= (1.0 - std::exp(m_restitution * inv_dt)) * scale;
			m_pixelToMeter = 1.0 / m_meterToPixel;

			if (m_targetBody != nullptr)
			{
				Vector2 real_origin(m_origin.x + m_transform.x, m_origin.y - m_transform.y);
				Vector2 target(-(m_targetBody->position().x + m_targetBody->shape()->center().x), m_targetBody->position().y + m_targetBody->shape()->center().y);
				target = worldToScreen(target) - real_origin;

				Vector2 c = target - m_transform;

				//lerp
				//if (c.lengthSquare() < 0.1)
				//	m_transform = target;
				//else
				//	m_transform += c * 0.02;

				//exp
				if (c.lengthSquare() < 0.1)
					m_transform = target;
				else
					m_transform -= (1.0 - std::exp(m_restitution * inv_dt)) * c;

				//real frames = m_easingTime * inv_dt;


			}

			QBrush bgBrush(QColor(50, 50, 50));
			//QBrush bgBrush(QColor(237, 237, 237));
			painter->setRenderHint(QPainter::Antialiasing);
			painter->setClipRect(m_viewport.topLeft.x, m_viewport.topLeft.y, m_viewport.width(), m_viewport.height());
			//painter->setBackground(QBrush(QColor(50, 50, 50)));
			painter->setBackground(bgBrush);

			painter->fillRect(QRectF(m_viewport.topLeft.x, m_viewport.topLeft.y, m_viewport.width(), m_viewport.height()),
				bgBrush);
			QPen origin(Qt::green, 10, Qt::SolidLine, Qt::RoundCap, Qt::RoundJoin);


			//RendererQtImpl::renderPoint(painter, this, Vector2(0, 0), origin);
			//QColor primary("#212121");
			QColor primary(Qt::green);
			QPen pen(primary, 1, Qt::SolidLine, Qt::RoundCap, Qt::RoundJoin);
			if (m_bodyVisible)
			{
				for (auto& body : m_world->bodyList())
				{
					ShapePrimitive primitive;
					primitive.shape = body->shape();
					primitive.rotation = body->rotation();
					primitive.transform = body->position();
					if (body->sleep())
						pen.setColor(Qt::gray);
					RendererQtImpl::renderShape(painter, this, primitive, pen);
					if (m_centerVisible)
					{
						QPen center(Qt::gray, 8, Qt::SolidLine, Qt::RoundCap, Qt::RoundJoin);
						RendererQtImpl::renderPoint(painter, this, primitive.transform, center);
					}
					if (m_rotationLineVisible)
						RendererQtImpl::renderAngleLine(painter, this, primitive);
				}
			}
			if (m_jointVisible)
			{
				for (auto& joint : m_world->jointList())
					if (joint->active())
						RendererQtImpl::renderJoint(painter, this, joint.get(), pen);
			}
			if (m_axisVisible)
			{
				QColor color = Qt::green;
				color.setAlphaF(0.8);
				pen.setColor(color);
				std::vector<Vector2> axisPoints;
				axisPoints.reserve(static_cast<size_t>(m_axisPointCount * 2 + 1));

				for (real i = -m_axisPointCount; i <= m_axisPointCount; i += 1.0)
				{
					axisPoints.emplace_back(Vector2(0, i));
					axisPoints.emplace_back(Vector2(i, 0));
				}
				RendererQtImpl::renderPoints(painter, this, axisPoints, pen);
				color = Qt::green;
				color.setAlphaF(0.7);
				pen.setColor(color);
				pen.setWidth(1);
				RendererQtImpl::renderLine(painter, this, Vector2(0, -m_axisPointCount), Vector2(0, m_axisPointCount), pen);
				RendererQtImpl::renderLine(painter, this, Vector2(-m_axisPointCount, 0), Vector2(m_axisPointCount, 0), pen);

			}
			if (m_aabbVisible)
			{
				QPen pen(Qt::cyan, 1, Qt::SolidLine, Qt::RoundCap, Qt::RoundJoin);
				for (auto [body, node] : m_dbvh->leaves())
					RendererQtImpl::renderAABB(painter, this, node->aabb, pen);
				for (auto& elem : m_tree->tree())
					if (elem.body != nullptr)
						RendererQtImpl::renderAABB(painter, this, elem.aabb, pen);
			}
			if (m_dbvhVisible)
			{
				DBVH::Node* root = m_dbvh->root();
				drawDbvh(root, painter);
			}
			if (m_treeVisible)
			{
				drawTree(m_tree->rootIndex(), painter);
			}
			if (m_gridScaleLineVisible)
			{
				drawGridScaleLine(painter);
			}
			if (m_contactVisible)
			{
				drawContacts(painter);
			}
		}
	}

	bool Camera::aabbVisible() const
	{
		return m_aabbVisible;
	}

	void Camera::setAabbVisible(bool aabbVisible)
	{
		m_aabbVisible = aabbVisible;
	}

	bool Camera::jointVisible() const
	{
		return m_jointVisible;
	}

	void Camera::setJointVisible(bool jointVisible)
	{
		m_jointVisible = jointVisible;
	}

	bool Camera::bodyVisible() const
	{
		return m_bodyVisible;
	}

	void Camera::setBodyVisible(bool bodyVisible)
	{
		m_bodyVisible = bodyVisible;
	}

	bool Camera::axisVisible() const
	{
		return m_axisVisible;
	}

	void Camera::setAxisVisible(bool axisVisible)
	{
		m_axisVisible = axisVisible;
	}

	bool Camera::gridScaleLineVisible() const
	{
		return m_gridScaleLineVisible;
	}

	void Camera::setGridScaleLineVisible(bool visible)
	{
		m_gridScaleLineVisible = visible;
	}

	real Camera::axisPointCount() const
	{
		return m_axisPointCount;
	}

	void Camera::setAxisPointCount(real count)
	{
		m_axisPointCount = count;
	}

	real Camera::meterToPixel() const
	{
		return m_meterToPixel;
	}

	void Camera::setMeterToPixel(const real& meterToPixel)
	{
		if (meterToPixel < 1.0)
		{
			m_targetMeterToPixel = 1.0;
			m_targetPixelToMeter = 1.0;
			return;
		}
		m_targetMeterToPixel = meterToPixel;
		m_targetPixelToMeter = 1.0f / meterToPixel;
	}

	Vector2 Camera::transform() const
	{
		return m_transform;
	}

	void Camera::setTransform(const Vector2& transform)
	{
		m_transform = transform;
	}

	void Camera::setWorld(PhysicsWorld* world)
	{
		m_world = world;
	}

	PhysicsWorld* Camera::world() const
	{
		return m_world;
	}

	Body* Camera::targetBody() const
	{
		return m_targetBody;
	}

	void Camera::setTargetBody(Body* targetBody)
	{
		m_targetBody = targetBody;
	}

	real Camera::zoomFactor() const
	{
		return m_zoomFactor;
	}

	void Camera::setZoomFactor(const real& zoomFactor)
	{
		m_zoomFactor = zoomFactor;
	}

	bool Camera::dbvhVisible() const
	{
		return m_dbvhVisible;
	}

	void Camera::setDbvhVisible(bool dbvhVisible)
	{
		m_dbvhVisible = dbvhVisible;
	}

	Camera::Viewport Camera::viewport() const
	{
		return m_viewport;
	}

	void Camera::setViewport(const Viewport& viewport)
	{
		m_viewport = viewport;
		m_origin.set((m_viewport.topLeft.x + m_viewport.bottomRight.x) * (0.5), (m_viewport.topLeft.y + m_viewport.bottomRight.y) * (0.5));
	}
	Vector2 Camera::worldToScreen(const Vector2& pos)const
	{
		Vector2 real_origin(m_origin.x + m_transform.x, m_origin.y - m_transform.y);
		return Vector2(real_origin.x + pos.x * m_meterToPixel, real_origin.y - pos.y * m_meterToPixel);
	}
	Vector2 Camera::screenToWorld(const Vector2& pos)const
	{
		Vector2 real_origin(m_origin.x + m_transform.x, m_origin.y - m_transform.y);
		Vector2 result = pos - real_origin;
		result.y = -result.y;
		result *= m_pixelToMeter;
		return result;
	}
	DBVH* Camera::dbvh() const
	{
		return m_dbvh;
	}

	void Camera::setDbvh(DBVH* dbvh)
	{
		m_dbvh = dbvh;
	}

	Tree* Camera::tree() const
	{
		return m_tree;
	}

	void Camera::setTree(Tree* tree)
	{
		m_tree = tree;
	}

	bool Camera::visible() const
	{
		return m_visible;
	}

	void Camera::setVisible(bool visible)
	{
		m_visible = visible;
	}

	bool Camera::treeVisible() const
	{
		return m_treeVisible;
	}

	void Camera::setTreeVisible(bool visible)
	{
		m_treeVisible = visible;
	}

	real Camera::deltaTime() const
	{
		return m_deltaTime;
	}

	void Camera::setDeltaTime(const real& deltaTime)
	{
		m_deltaTime = deltaTime;
	}

	bool Camera::rotationLineVisible() const
	{
		return m_rotationLineVisible;
	}

	void Camera::setRotationLineVisible(bool visible)
	{
		m_rotationLineVisible = visible;
	}

	bool Camera::centerVisible() const
	{
		return m_centerVisible;
	}

	void Camera::setCenterVisible(bool visible)
	{
		m_centerVisible = visible;
	}

	bool Camera::contactVisible() const
	{
		return m_contactVisible;
	}

	void Camera::setContactVisible(bool visible)
	{
		m_contactVisible = visible;
	}

	ContactMaintainer* Camera::maintainer() const
	{
		return m_maintainer;
	}

	void Camera::setContactMaintainer(ContactMaintainer* maintainer)
	{
		m_maintainer = maintainer;
	}

	void Camera::drawTree(int nodeIndex, QPainter* painter)
	{
		if (nodeIndex == -1)
			return;

		drawTree(m_tree->tree()[nodeIndex].leftIndex, painter);
		drawTree(m_tree->tree()[nodeIndex].rightIndex, painter);

		QPen pen(Qt::cyan, 1, Qt::SolidLine, Qt::RoundCap, Qt::RoundJoin);
		AABB aabb = m_tree->tree()[nodeIndex].aabb;
		//aabb.expand(0.5);
		if (!m_tree->tree()[nodeIndex].isLeaf())
			RendererQtImpl::renderAABB(painter, this, aabb, pen);
	}
	void Camera::drawContacts(QPainter* painter)
	{
		QColor colorApproximate3(Qt::magenta);
		QColor colorApproximate4(Qt::yellow);
		colorApproximate3.setAlphaF(0.5);
		colorApproximate4.setAlphaF(0.5);
		QPen penApproximate3(colorApproximate3, 2, Qt::SolidLine, Qt::RoundCap, Qt::RoundJoin);
		QPen penApproximate4(colorApproximate4, 2, Qt::SolidLine, Qt::RoundCap, Qt::RoundJoin);
		for (auto iter = m_maintainer->m_contactTable.begin(); iter != m_maintainer->m_contactTable.end(); ++iter)
		{
			for (auto& elem : iter->second)
			{
				RendererQtImpl::renderPoint(painter, this, elem.bodyA->toWorldPoint(elem.localA), penApproximate4);
				RendererQtImpl::renderPoint(painter, this, elem.bodyB->toWorldPoint(elem.localB), penApproximate3);
			}
		}
	}
	void Camera::drawGridScaleLine(QPainter* painter)
	{
		QColor color = Qt::darkGreen;
		QPen pen;
		color.setAlphaF(0.8);
		pen.setColor(color);
		pen.setWidth(1);
		bool fineEnough = m_meterToPixel > 180;
		real h;

		if (m_meterToPixel < 30)
			h = 10.0;
		else if (m_meterToPixel < 60)
			h = 5.0;
		else if (m_meterToPixel < 120)
			h = 2.0;
		else
			h = 1.0;

		std::vector<std::pair<Vector2, Vector2>> lines;
		lines.reserve(static_cast<size_t>(m_axisPointCount * 2));
		for (real i = -m_axisPointCount; i <= m_axisPointCount; i += h)
		{
			if (realEqual(i, 0.0))
				continue;
			Vector2 p1 = { i, m_axisPointCount };
			Vector2 p2 = { i, -m_axisPointCount };
			lines.emplace_back(std::make_pair(p1, p2));

			p1.set(-m_axisPointCount, i);
			p2.set(m_axisPointCount, i);
			lines.emplace_back(std::make_pair(p1, p2));
		}
		RendererQtImpl::renderLines(painter, this, lines, pen);
		if (fineEnough)
		{
			if (m_meterToPixel < 400)
				h = 0.2f;
			else if (m_meterToPixel < 800)
				h = 0.1f;
			else if (m_meterToPixel < 1600)
				h = 0.05f;
			else if (m_meterToPixel < 4000)
				h = 0.02f;
			else
				h = 0.005f;

			lines.clear();
			lines.reserve(static_cast<size_t>(m_axisPointCount * 2.0f / 0.2f));
			color.setAlphaF(0.3);
			pen.setColor(color);
			for (real i = -m_axisPointCount; i <= m_axisPointCount; i += h)
			{
				if (realEqual(i, 0.0f))
					continue;
				if (realEqual(i - std::floor(i), 0.0f))
					continue;
				Vector2 p1 = { i, m_axisPointCount };
				Vector2 p2 = { i, -m_axisPointCount };
				lines.emplace_back(std::make_pair(p1, p2));

				p1.set(-m_axisPointCount, i);
				p2.set(m_axisPointCount, i);
				lines.emplace_back(std::make_pair(p1, p2));
			}
			RendererQtImpl::renderLines(painter, this, lines, pen);
		}
	}
	void Camera::drawDbvh(DBVH::Node* node, QPainter* painter)
	{
		if (node == nullptr)
			return;

		drawDbvh(node->left, painter);
		drawDbvh(node->right, painter);

		QPen pen(Qt::cyan, 1, Qt::SolidLine, Qt::RoundCap, Qt::RoundJoin);
		AABB aabb = node->aabb;
		if (!node->isLeaf())
			RendererQtImpl::renderAABB(painter, this, aabb, pen);
	}
	real Camera::Viewport::width()
	{
		return bottomRight.x - topLeft.x;
	}
	real Camera::Viewport::height()
	{
		return bottomRight.y - topLeft.y;
	}
	void Camera::Viewport::setWidth(const real& width)
	{
		bottomRight.x = topLeft.x + width;
	}
	void Camera::Viewport::setHeight(const real& height)
	{
		bottomRight.y = topLeft.y + height;
	}
	void Camera::Viewport::set(const real& width, const real& height)
	{
		setWidth(width);
		setHeight(height);
	}
}