GameObject* CameraComponent::Pick(const GameContext& gameContext, CollisionGroupFlag ignoreGroups) const
{
	// get mouse position
	auto mousepos = gameContext.pInput->GetMousePosition();
	// get half of the window size
	float halfWidth = OverlordGame::GetGameSettings().Window.Width / 2.0f;
	float halfHeight = OverlordGame::GetGameSettings().Window.Height / 2.0f;

	XMFLOAT3 nearPoint, farPoint;
	// get the mouse position in NDC
	XMFLOAT2 mousePosNDC = XMFLOAT2{ ((float)mousepos.x - halfWidth) / halfWidth , (halfHeight - (float)mousepos.y) / halfHeight };
	// get the far and near points
	auto tempnearPoint = XMFLOAT3(mousePosNDC.x, mousePosNDC.y, 0);
	auto tempfarPoint = XMFLOAT3(mousePosNDC.x, mousePosNDC.y, 1);
	// get the far and near vectors
	XMVECTOR nearPointVec = XMVector3TransformCoord(XMLoadFloat3(&tempnearPoint), XMLoadFloat4x4(&m_ViewProjectionInverse));
	XMStoreFloat3(&nearPoint, nearPointVec);

	XMVECTOR farPointVec = XMVector3TransformCoord(XMLoadFloat3(&tempfarPoint), XMLoadFloat4x4(&m_ViewProjectionInverse));
	XMStoreFloat3(&farPoint, farPointVec);
	// set the ignore groups
	PxQueryFilterData filterdata;
	filterdata.data.word0 = ~ignoreGroups;

	PxRaycastBuffer hit;
	// get the physXProxy
	auto physXProxy = GetGameObject()->GetScene()->GetPhysxProxy();
	
	// get the direction for the raycast
	XMFLOAT3 direction;
	XMStoreFloat3(&direction, farPointVec - nearPointVec);
	auto rayStart = PxVec3(nearPoint.x, nearPoint.y, nearPoint.z);
	auto rayDirection = PxVec3(direction.x, direction.y, direction.z);
	rayDirection.normalize();
	// execute raycast
	physXProxy->Raycast(rayStart, rayDirection, PX_MAX_F32, hit, PxHitFlag::eDEFAULT, filterdata);
	// check if there was a hit
	BaseComponent* component = nullptr;
		if (hit.hasBlock)
		{
			// return the hit actor
			component = (BaseComponent*)hit.block.actor->userData;
			return component->GetGameObject();
		}
	return nullptr;
}