이 프로젝트는 FPS 게임에서 사용되는 무기시스템, 간단한 인벤토리 시스템, 멀티플레이시 실시간 네트워크 동기화를 사용해 구현한 예제입니다. 그레네이드의 궤적 계산, 충돌 처리, 미사일의 타격감과 물리적 상호작용을 포함하며, 서버와 클라이언트 간의 동기화 문제를 해결하기 위한 네트워크 코드도 포함되어 있습니다.

## 주요 기능
- **Rifle, Shotgun 무기 구현**: 플레이어가 소총 및 샷건을 사용할 때의 발사 메커니즘과 타격 판정을 구현하고, 서버와 클라이언트 간의 발사 결과를 동기화하는 기능.
- **LandMine 지뢰 구현**: 플레이어가 지뢰를 설치할 수 있는 기능을 구현하였으며, 적이 지뢰에 접근하거나 밟을 때 발생하는 폭발 및 데미지를 처리하는 시스템.
- **GrenadeLauncher 궤적 계산 및 범위 데미지**: 그레네이드 런처에서 발사된 투사체의 궤적을 물리 엔진을 통해 계산하고, 폭발 범위 내 적에게 데미지를 입히는 시스템을 구현. 이를 서버와 클라이언트 간에 동기화하여 모든 플레이어가 동일한 결과를 경험하도록 처리.
- **Missile 충돌 처리 및 유도기능 구현**: 미사일이 적 또는 물체에 충돌할 때 발생하는 충돌 처리와 폭발 이펙트를 구현했으며, 유도 기능을 통해 미사일이 목표물을 추적하는 시스템을 추가.
- **실시간 네트워크 동기화**: 서버와 클라이언트 간에 무기 및 투사체 데이터를 동기화하여, 모든 플레이어에게 일관되고 매끄러운 멀티플레이 경험을 제공.


- **Missile 충돌 처리 및 유도기능 구현**
  Missile 발사 전 Target을 지정하여 Actor가 움직이더라도 따라가는 유도기능 구현.



![Rifle Example](./Giffile/Rifle.gif)

![Shotgun Example](./Giffile/Shotgun.gif)

![Inventory Example](./Giffile/Inventoryin.gif)

![Inventory Example](./Giffile/Inventoryout.gif)

![GrenadeLauncher Example](./Giffile/GrenadeLauncher.gif)

![Missile Example](./Giffile/Missile.gif)
