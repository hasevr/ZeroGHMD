#include <conio.h>
#include <Springhead.h>
#include <HumanInterface/SprHIDRUsb.h>
#include <iomanip>

using namespace Spr;

int __cdecl main() {

	// HumanInterface SDKを作成
	UTRef<HISdkIf> hiSdk = HISdkIf::CreateSdk();

	// win32
	DRUsb20SimpleDesc usbSimpleDesc;
	hiSdk->AddRealDevice(DRUsb20SimpleIf::GetIfInfoStatic(), &usbSimpleDesc);
	DRUsb20Sh4Desc usb20Sh4Desc;
	for (int i = 0; i < 10; ++i) {
		usb20Sh4Desc.channel = i;
		hiSdk->AddRealDevice(DRUsb20Sh4If::GetIfInfoStatic(), &usb20Sh4Desc);
	}
	// win64
	DRCyUsb20Sh4Desc cyDesc;
	for (int i = 0; i < 10; ++i) {
		cyDesc.channel = i;
		hiSdk->AddRealDevice(DRCyUsb20Sh4If::GetIfInfoStatic(), &cyDesc);
	}
	//	UART Motor Driver
	DRUARTMotorDriverDesc umDesc;
	DRUARTMotorDriverIf* uartMotorDriver = hiSdk->AddRealDevice(DRUARTMotorDriverIf::GetIfInfoStatic(), &umDesc)->Cast();
	hiSdk->AddRealDevice(DRKeyMouseWin32If::GetIfInfoStatic());
	hiSdk->Print(DSTR);
	hiSdk->Print(std::cout);

	// Spidarインタフェース作成
	UTRef<HISpidar4If> spg = hiSdk->CreateHumanInterface(HISpidar4If::GetIfInfoStatic())->Cast();
	HISpidar4Desc desc("SpidarG6X3R");
	for (int i = 0; i < desc.motors.size(); ++i) {
		desc.motors[i].lengthPerPulse *= 1.66;
	}

	//	左後エアコン：203cm  ポール左前 223㎝-10cm　　ポール右前 220-10cm	右後ろ壁　222-10cm
	//	151(パイプ-壁）		151-33(エアコン-壁)=118		左右ポール110		47+74=121cm 後ろ左右	
	//	  H213           H210
	//	  +---------------+
	//    |       110     |
	//	  118           151
	//	  |               |
	//33  +H203----+      |
	//  +-+--------+------+ H212
	//  +       121       +
	const double lz = 1.51 / 2;
	const double yHmd = 0.20;
	desc.motors[0].pos = Vec3f(0.55f, 2.14-yHmd, -lz);			//	RF
	desc.motors[1].pos = Vec3f(-0.55f, 2.14-yHmd, -lz);			//	LF
	desc.motors[2].pos = Vec3f(-(0.55f+0.11f), 2.03-yHmd, lz-0.33);	//	LB
	desc.motors[3].pos = Vec3f(0.55f, 2.12-yHmd, lz);			//	RB

	if (!spg->Init(&desc)) exit(-1);

	int t = 0;
	for (int i = 0; i < spg->NMotor(); ++i) {
		spg->GetMotor(i)->SetLimitMinForce(0.01);
	}
	Vec3f force(0.0f, 1.0f, 0.0f);
	while (1){
		if (_kbhit()) {
			char ch = _getch();
			switch (ch) {
			case 'p':
				force.y += 0.1f;
				break;
			case 'l':
				force.y -= 0.1f;
				if (force.y < 1.0) force.y = 1.0;
				break;
			case 'c':
				spg->Calibration();
				std::cout << "Calibration" << std::endl;
				break;
			case 'q':
			case 'Q':
			case '0x1b':
				goto next;
			}
		}
		t += 1;
#if 1
		Vec3f spgpos = spg->GetPosition();
		if (t % 1000 == 1) {
			std::cout << std::setprecision(2) << "p:" << spgpos << " f:" << force << std::endl;
		}
		spg->SetForce(force);
#else if
		for (size_t i = 0; i < spg->NMotor(); ++i) {
			std::cout << " " << std::setprecision(4) << spg->GetMotor(i)->GetLength();
		}
//		std::cout << minForce;
		std::cout << std::endl;
#endif
		spg->Update(0.001f);
	}
next:;
	spg->SetForce(Vec3f(0,0,0));
	for (int i = 0; i < spg->NMotor(); ++i) {
		spg->GetMotor(i)->SetLimitMinForce(0.3f);
	}
	spg->Update(0.001f);
	/*
		DRKeyMouseWin32If* wif = DCAST(DRKeyMouseWin32If, sdk->FindRealDevice("KeyMouseWin32"));
		wif->Update();
		UTRef<HIMouse6DIf> mouse6D = DCAST(HIMouse6DIf, sdk->CreateHumanInterface("HIMouse6D", NULL));
		while(1){
			if (mouse6D->GetKeyMouse()->GetKeyState('Q') & DVKeyMouseIf::PRESS) return 0;
			for(int i=0; i<200; ++i){
				if (mouse6D->GetKeyMouse()->GetKeyState(i) & DVKeyMouseIf::PRESS){
					std::cout << i << " '"<< (char)i << "' " << std::endl;
				}
			}
		}
	*/
	return 0;
}