// ConsoleApplication1.cpp : Defines the entry point for the console application.
//

#include "stdafx.h"

#include <stdio.h>
#include <stdlib.h>
#include <gl/glut.h>
#include <PxPhysicsAPI.h>
#include "Targa.h"

#pragma comment(lib, "PhysX3_x86.lib")
#pragma comment(lib, "PhysX3Common_x86.lib") 
#pragma comment(lib, "PhysX3Extensions.lib")
#pragma comment(lib, "PhysXProfileSDKCHECKED.lib")

using namespace physx;

#define ANIM_FPS	40		// Docelowa liczba ramek animacji na sekund
#define SIM_TIME	1/30.0	// Jeden krok silnika fizycznego (1/60 to domyslny krok)

PxVec3 _force_vec = PxVec3(0, 0, -180);

// rozmiary bryły obcinania
const GLdouble left = -2.0;
const GLdouble right = 2.0;
const GLdouble bottom = -2.0;
const GLdouble top = 2.0;
GLuint tekstura[1];
GLuint tekstura1[1];

// kąty obrotu
GLfloat rotatex = 50.0;
GLfloat rotatey = 50.0;
int button1;

// wskaźnik naciśnięcia lewego przycisku myszki
int button_state = GLUT_UP;

// położenie kursora myszki
int button_x, button_y;

// współczynnik skalowania
GLfloat scale = 1.0;

// Globalne zmienne:
PxFoundation *pfund;		// Fundament (obowiazkowy w PhysX3)
PxPhysics* pphys;			// PhysX SDK
PxScene* gsc;				// Opis sceny
PxVec3 gg(0, -10, 0);		// Wektor grawitacji (m/s2)

// G��wni aktorzy tej sceny (PhysX):
PxRigidStatic* a_plane;		// Statyczna pod�oga
PxRigidDynamic* a_box;		// Dynamiczny sze�cian (klocek)
PxRigidDynamic* a_sfera;		// Dynamiczna sfera 
PxRigidDynamic* a_sfera2;		// Dynamiczna sfera 
PxRigidDynamic* a_sfera3;		// Dynamiczna sfera 
PxRigidDynamic* a_sfera4;		// Dynamiczna sfera 
PxRigidDynamic* a_sfera5;		// Dynamiczna sfera 
PxRigidDynamic* a_sfera6;		// Dynamiczna sfera 
PxRigidDynamic* a_sferaBiala;		// Dynamiczna sfera 
PxRigidStatic* a_plane1;		// Statyczna pod�oga
PxRigidStatic* a_plane2;		// Statyczna pod�oga
PxRigidStatic* a_plane3;		// Statyczna pod�oga
PxRigidStatic* a_plane4;		// Statyczna pod�oga


void tekstury(void) {
	glEnable(GL_TEXTURE_2D);

	//mieszanie kolorow tekstury z obiektem (?)
	glTexEnvf(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_REPLACE);
	//mipmapy gdy tekstura jest mala
	glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_NEAREST);
	//gdy tekstura jest powiekszona - stosowanie filtru bilinearnego
	glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	//powtarzanie tekstur wokol osi
	glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
	glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);

	/*	if (LoadTGATexture("plyta6.tga") == false) {
	printf("Blad wczytania tekstury czasteczka.tga");
	}*/

}


GLubyte *LoadTGAImage(char *filename, TARGAINFO *info)
{
	GLubyte	TGAHeader[12] = { 0, 0, 2, 0, 0, 0, 0, 0, 0, 0, 0, 0 };	// Nagłówek TGA bez kompresji
	GLubyte	TGACompare[12];			// Tu się załaduje dane z pliku
	GLubyte	Header[6];			// Pierwsze potrzebne 6 bajtów z pliku
	GLubyte *Bits = NULL;	// Wskaźnik na bufor z danymi pikseli

	FILE *plik = fopen(filename, "rb");	// Próba otwarcia do odczytu
	if (plik)
	{
		fread(TGACompare, 1, sizeof(TGACompare), plik);	// Odczytanie nagłówka pliku
		if (memcmp(TGAHeader, TGACompare, sizeof(TGAHeader)) == 0)	// Nagłówek identyczny?
		{
			fread(Header, 1, sizeof(Header), plik);	// Odczyt użytecznych danych

			// Wyłuskanie informacji o rozmiarach
			info->width = Header[1] * 256 + Header[0];
			info->height = Header[3] * 256 + Header[2];
			info->bpp = Header[4];

			// Sprawdzenie czy rozmiary > 0 oraz czy bitmapa 24 lub 32-bitowa
			if (info->width>0 && info->height>0 && (info->bpp == 24 || info->bpp == 32))
			{
				long ImageSize = info->height * info->width * info->bpp / 8;	// Obliczenie ilości danych
				Bits = (GLubyte*)malloc(ImageSize);	// Alokacja pamięci na dane

				if (Bits)
				{
					fread(Bits, 1, ImageSize, plik);	// Odczyt właściwych danych pikseli z pliku

					// Konwersja BGR -> RGB
					int i;
					GLubyte tmp;	// Miejsce przechowania jednej wartości koloru

					for (i = 0; i < ImageSize; i += info->bpp / 8)	// Wszystkie wartości RGB lub RGBA
					{
						tmp = Bits[i];
						Bits[i] = Bits[i + 2];
						Bits[i + 2] = tmp;
					}
				}
			}
		}

		fclose(plik);
	}

	return(Bits);
}

///////////////////////////////////////////////////////////////////////////////
// Procedury na podstawie przekazanych danych ładują
// i tworzą teksturę lub teksturę z Mip-Map'ami
bool LoadTGATexture(char *filename)
{
	TARGAINFO info;	// Dane o bitmapie
	GLubyte *bits;	// Dane o pikselach

	// ładowanie pliku
	bits = LoadTGAImage(filename, &info);	// Próba wczytania tekstury
	if (bits == NULL)	return(FALSE);	// ERROR

	// Ustawienie parametrów tekstury
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);

	glPixelStorei(GL_UNPACK_ALIGNMENT, 4);

	if (info.bpp == 24)	// Bitmapa z danymi RGB
		glTexImage2D(GL_TEXTURE_2D, 0, 3, info.width, info.height, 0, GL_RGB, GL_UNSIGNED_BYTE, bits);
	else	// Bitmapa z danymi RGBA
		glTexImage2D(GL_TEXTURE_2D, 0, 4, info.width, info.height, 0, GL_RGBA, GL_UNSIGNED_BYTE, bits);

	free(bits);
	return(TRUE);
}

bool LoadTGAMipmap(char *filename)
{
	TARGAINFO info;	// Dane o bitmapie
	GLubyte *bits;	// Dane o pikselach

	// ładowanie pliku
	bits = LoadTGAImage(filename, &info);	// Próba wczytania tekstury
	if (bits == NULL)	return(FALSE);	// ERROR

	// Ustawienie parametrów tekstury
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST_MIPMAP_LINEAR);

	glPixelStorei(GL_UNPACK_ALIGNMENT, 4);

	if (info.bpp == 24)	// Bitmapa z danymi RGB
		gluBuild2DMipmaps(GL_TEXTURE_2D, 3, info.width, info.height, GL_RGB, GL_UNSIGNED_BYTE, bits);
	else	// Bitmapa z danymi RGBA
		gluBuild2DMipmaps(GL_TEXTURE_2D, 4, info.width, info.height, GL_RGBA, GL_UNSIGNED_BYTE, bits);

	free(bits);
	return(TRUE);
}


// Funkcje tworzace obiekty (aktorow PhysX):
PxRigidStatic* make_plane(PxMaterial *mat) {
	// Pozioma pod�oga:
	PxTransform pose = PxTransform(PxVec3(0, 0, 0), PxQuat(PxHalfPi, PxVec3(0, 0, 1)));
	// Utworzenie aktora statycznego:
	PxRigidStatic *plane = pphys->createRigidStatic(pose);
	// Utworzenie kszta�tu i przypisanie do aktora:
	PxShape* shape = plane->createShape(PxPlaneGeometry(), *mat);
	// Utworzenie aktora:
	gsc->addActor(*plane);
	return(plane);
}

PxRigidStatic* make_boxsciana1(PxMaterial *mat) {
	// Rotacja w przestrzeni (�eby klocek nie spada� p�asko):
	
	PxTransform transform(PxVec3(-10.0f, 0.0f, 0.0f));
	// Rozmiary klocka:
	PxVec3 dimensions(0.1, 2, 20);
	PxBoxGeometry geometry(dimensions);
	// Utworzenie dynamicznego aktora:
	PxRigidStatic *actor = PxCreateStatic(*pphys, transform, geometry, *mat);
	//actor->setAngularDamping(0.75);
	gsc->addActor(*actor);

	return(actor);
}
PxRigidStatic* make_boxsciana2(PxMaterial *mat) {
	// Rotacja w przestrzeni (�eby klocek nie spada� p�asko):
	
	PxTransform transform(PxVec3(10.0f, 0.0f, 0.0f));
	// Rozmiary klocka:
	PxVec3 dimensions(0.1, 2, 20);
	PxBoxGeometry geometry(dimensions);
	// Utworzenie dynamicznego aktora:
	PxRigidStatic *actor = PxCreateStatic(*pphys, transform, geometry, *mat);
	//actor->setAngularDamping(0.75);
	gsc->addActor(*actor);

	return(actor);
}
PxRigidStatic* make_boxsciana3(PxMaterial *mat) {
	// Rotacja w przestrzeni (�eby klocek nie spada� p�asko):

	PxTransform transform(PxVec3(0.0f, 0.0f, 20.0f));
	// Rozmiary klocka:
	PxVec3 dimensions(20, 2, 0.1);
	PxBoxGeometry geometry(dimensions);
	// Utworzenie dynamicznego aktora:
	PxRigidStatic *actor = PxCreateStatic(*pphys, transform, geometry, *mat);
	//actor->setAngularDamping(0.75);
	gsc->addActor(*actor);

	return(actor);
}
PxRigidStatic* make_boxsciana4(PxMaterial *mat) {
	// Rotacja w przestrzeni (�eby klocek nie spada� p�asko):

	PxTransform transform(PxVec3(0.0f, 0.0f, -20.0f));
	// Rozmiary klocka:
	PxVec3 dimensions(20, 2, 0.1);
	PxBoxGeometry geometry(dimensions);
	// Utworzenie dynamicznego aktora:
	PxRigidStatic *actor = PxCreateStatic(*pphys, transform, geometry, *mat);
	//actor->setAngularDamping(0.75);
	gsc->addActor(*actor);

	return(actor);
}
PxRigidDynamic* make_box(PxMaterial *mat) {
	PxReal density = 1.0f;
	// Rotacja w przestrzeni (�eby klocek nie spada� p�asko):
	PxQuat kwa(3.1415 / 3, PxVec3(1, 0, 0));
	PxTransform transform(PxVec3(0.0f, 10.0f, 0.0f), kwa);
	// Rozmiary klocka:
	PxVec3 dimensions(0.5, 0.5, 0.5);
	PxBoxGeometry geometry(dimensions);
	// Utworzenie dynamicznego aktora:
	PxRigidDynamic *actor = PxCreateDynamic(*pphys, transform, geometry, *mat, density);
	actor->setAngularDamping(0.75);
	gsc->addActor(*actor);

	return(actor);
}

PxRigidDynamic* make_sfera(PxMaterial *mat, float radius, float x, float y, float z) {
	// Położenie sfery:
	PxTransform transform(PxVec3(x, y, z));
	// Kształt sfery o podanym promieniu:
	PxSphereGeometry geometry(radius);
	// Utworzenie dynamicznego aktora:
	PxRigidDynamic *actor = PxCreateDynamic(*pphys,
		transform, geometry, *mat, 1.0f);
	gsc->addActor(*actor);
	return(actor);
}

PxRigidDynamic* make_sferaBiala(PxMaterial *mat, float radius, float x, float y, float z) {
	// Położenie sfery:
	PxTransform transform(PxVec3(x, y, z));
	// Kształt sfery o podanym promieniu:
	PxSphereGeometry geometry(radius);
	// Utworzenie dynamicznego aktora:
	PxRigidDynamic *actor = PxCreateDynamic(*pphys,
		transform, geometry, *mat, 1.0f);
	gsc->addActor(*actor);
	return(actor);
}

// Domy�lna obs�uga b��d�w:
PxDefaultErrorCallback gDefaultErrorCallback;
// Domy�lna obs�uga alokacji pami�ci:
PxDefaultAllocator gDefaultAllocatorCallback;

void init_physx() {
	// Obiekt klasy PxFoundation:
	pfund = PxCreateFoundation(PX_PHYSICS_VERSION, gDefaultAllocatorCallback, gDefaultErrorCallback);
	if (!pfund) {
		puts("PxCreateFoundation kaput!");
		exit(0);
	}
	// Tworzenie SDK, domy�lne ustawienia materia��w:
	pphys = PxCreatePhysics(PX_PHYSICS_VERSION, *pfund, PxTolerancesScale());
	if (!pphys) {
		puts("PxCreatePhysics kaput!");
		exit(0);
	}

	// Utworzenie "sceny":
	PxSceneDesc sceneDesc(pphys->getTolerancesScale());
	sceneDesc.gravity = gg;

	// Jeden w�tek domy�lnego ekspedytora (dispatcher):
	PxDefaultCpuDispatcher *cpudisp = PxDefaultCpuDispatcherCreate(1);
	if (!cpudisp) {
		puts("PxDefaultCpuDispatcherCreate kaput!");
		exit(0);
	}
	sceneDesc.cpuDispatcher = cpudisp;

	// Domy�lny filtr symuluj�cy dzia�anie biblioteki z wersji 2.8.x:
	sceneDesc.filterShader = &PxDefaultSimulationFilterShader;

	gsc = pphys->createScene(sceneDesc);
	if (!gsc) {
		puts("createScene kaput!");
		exit(0);
	}

	// Materia� na wszystkie obiekty:
	PxMaterial *mat = pphys->createMaterial(0.5f, 0.5f, 0.5f);    // static friction, dynamic friction, restitution
	
	// Utworzenie:
	PxMaterial *mat2 = pphys->createMaterial(1.0f, 1.0f, 0.0f);    // static friction, dynamic friction, restitution
	
	a_plane = make_plane(mat2);

	a_sfera = make_sfera(mat, 1, 0, 0, -5);
	a_sfera2 = make_sfera(mat, 1, -1, 0, -6.8);
	a_sfera3 = make_sfera(mat, 1, 1, 0, -6.8);
	a_sfera4 = make_sfera(mat, 1, -2, 0, -8.6);
	a_sfera5 = make_sfera(mat, 1, 0, 0, -8.6);
	a_sfera6 = make_sfera(mat, 1, 2, 0, -8.6);

	a_sferaBiala = make_sferaBiala(mat, 1, 0, 0, 12);

	a_plane1 = make_boxsciana1(mat2);
	a_plane2 = make_boxsciana2(mat2);
	a_plane3 = make_boxsciana3(mat2);
	a_plane4 = make_boxsciana4(mat2);

	// Zainicjowanie pierwszej klatki animacji (symulacji):
	gsc->simulate(SIM_TIME);
}

// Zamkni�cie i sprz�tni�cie biblioteki PhysX:
void kill_physx() {
	if (pphys != NULL) {
		if (gsc != NULL)		gsc->release();
		pphys->release();
	}
	if (pfund != NULL)	pfund->release();
}

// By Muhammad Mobeen Movania (http://mmmovania.blogspot.com)
void getColumnMajor(PxMat33 m, PxVec3 t, float* mat) {
	mat[0] = m.column0[0];
	mat[1] = m.column0[1];
	mat[2] = m.column0[2];
	mat[3] = 0;

	mat[4] = m.column1[0];
	mat[5] = m.column1[1];
	mat[6] = m.column1[2];
	mat[7] = 0;

	mat[8] = m.column2[0];
	mat[9] = m.column2[1];
	mat[10] = m.column2[2];
	mat[11] = 0;

	mat[12] = t[0];
	mat[13] = t[1];
	mat[14] = t[2];
	mat[15] = 1;
}

// Wygenerowanie maierzy przekszta�ce� dla OpenGL:
void SetupGLMatrix(const PxTransform &pose) {
	float glmat[16];	// Macierz 4x4 dla OpenGL
	// Pobranie macierzy przekszta�ce� i wektora translacji:
	PxMat33 m = PxMat33(pose.q);
	getColumnMajor(m, pose.p, glmat);
	// Do��czenie aktualnego przekszta�cenia do macierzy widoku modelu OpenGL:
	glMultMatrixf(&(glmat[0]));
}

void drawTable() {

	glEnable(GL_TEXTURE_2D);
	glAlphaFunc(GL_ALWAYS, 1.0f);
	glEnable(GL_ALPHA_TEST);
	LoadTGATexture("table.tga");
	glBindTexture(GL_TEXTURE_2D, tekstura[0]);

	glColor3f(0, 0.25, 0);
	glBegin(GL_QUADS);
	glTexCoord2f(1.0, 0.0);
	glVertex3f(-10, 0, -20);
	glTexCoord2f(0.0, 0.0);
	glVertex3f(-10, 0, 20);
	glTexCoord2f(0.0, 1.0);
	glVertex3f(10, 0, 20);
	glTexCoord2f(1.0, 1.0);
	glVertex3f(10, 0, -20);
	glEnd();
	glDisable(GL_TEXTURE_2D);

	glColor3f(0.0, 0.1, 0.0);
	glBegin(GL_QUADS);
	glVertex3f(-10, 0, -20);
	glVertex3f(-10, 0, 20);
	glVertex3f(-10, 2, 20);
	glVertex3f(-10, 2, -20);
	glEnd();

	glBegin(GL_QUADS);
	glVertex3f(10, 0, -20);
	glVertex3f(10, 0, 20);
	glVertex3f(10, 2, 20);
	glVertex3f(10, 2, -20);
	glEnd();

	glBegin(GL_QUADS);
	glVertex3f(-10, 0, 20);
	glVertex3f(10, 0, 20);
	glVertex3f(10, 2, 20);
	glVertex3f(-10, 2, 20);
	glEnd();

	//glColor3f(0.6, 0.3, 0.05);
	glBegin(GL_QUADS);
	glVertex3f(-10, 0, -20);
	glVertex3f(10, 0, -20);
	glVertex3f(10, 2, -20);
	glVertex3f(-10, 2, -20);
	glEnd();
}

void drawBalls() {
	//sfera begin
	PxTransform pose2 = a_sfera->getGlobalPose();	// Aktualne po�o�enie
	glPushMatrix();
	SetupGLMatrix(pose2);
	glColor3f(1, 0, 0);
	glutSolidSphere(/*size*/1, 15, 15);
	glPopMatrix();
	//sfera end


	//sfera2 begin
	PxTransform pose3 = a_sfera2->getGlobalPose();	// Aktualne po�o�enie
	glPushMatrix();
	SetupGLMatrix(pose3);
	glColor3f(0, 1, 0);
	glutSolidSphere(/*size*/1, 15, 15);
	glPopMatrix();
	//sfera2 end

	PxTransform pose4 = a_sfera3->getGlobalPose();	// Aktualne po�o�enie
	glPushMatrix();
	SetupGLMatrix(pose4);
	glColor3f(0, 0, 0);
	glutSolidSphere(/*size*/1, 15, 15);
	glPopMatrix();

	PxTransform pose5 = a_sferaBiala->getGlobalPose();	// Aktualne po�o�enie
	glPushMatrix();
	SetupGLMatrix(pose5);
	glColor3f(1, 1, 1);
	glutSolidSphere(/*size*/1, 15, 15);
	glPopMatrix();

	PxTransform pose6 = a_sfera4->getGlobalPose();	// Aktualne po�o�enie
	glPushMatrix();
	SetupGLMatrix(pose6);
	glColor3f(0, 0, 1);
	glutSolidSphere(/*size*/1, 15, 15);
	glPopMatrix();

	PxTransform pose7 = a_sfera5->getGlobalPose();	// Aktualne po�o�enie
	glPushMatrix();
	SetupGLMatrix(pose7);
	glColor3f(1, 1, 0);
	glutSolidSphere(/*size*/1, 15, 15);
	glPopMatrix();

	PxTransform pose8 = a_sfera6->getGlobalPose();	// Aktualne po�o�enie
	glPushMatrix();
	SetupGLMatrix(pose8);
	glColor3f(1, 0.5, 0);
	glEnable(GL_TEXTURE_2D);
	glAlphaFunc(GL_ALWAYS, 1.0f);
	glEnable(GL_ALPHA_TEST);
	LoadTGATexture("bila88.tga");
	glBindTexture(GL_TEXTURE_2D, tekstura1[0]);
	glutSolidSphere(/*size*/1, 15, 15);
	glDisable(GL_TEXTURE_2D);
	glPopMatrix();
}

// Gowwna funkcja rysujaca:
void RenderScene() {

	// kolor tła
	glClearColor(0.4, 0.7, 0.9, 1.0);

	// Wyczyszczenie okna i bufor�w (OpenGL):
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	glLoadIdentity();

	// Pobranie wynik�w symulacji (PhysX):
	gsc->fetchResults(true);

	// RENDERING (OpenGL):
	glTranslatef(0, 0, -35);
	glRotatef(rotatex, 1, 0, 0);
	glRotatef(rotatey, 0, 1, 0);

	// skalowanie obiektu - klawisze "+" i "-"
	glScalef(scale, scale, scale);

	drawTable();
	drawBalls();

	// Wyliczenie kolejnej klatka symulacji (PhysX):
	gsc->simulate(SIM_TIME);

	// skierowanie poleceń do wykonania
	glFlush();

	// Zmiana bufor�w (OpenGL):
	glutSwapBuffers();
}

// Zmiana wielko�ci okna i ustawienie rzutowania (OpenGL):
void ChangeSize(int w, int h) {
	// Nie dziel przez zero:
	if (h == 0)	h = 1;
	// Ustawienie widoku na ca�e okno:
	glViewport(0, 0, w, h);

	// Rzutowanie perspektywiczne:
	glMatrixMode(GL_PROJECTION);
	glLoadIdentity();
	gluPerspective(40, (float)w / h, 1, 100);

	// Wyzerowanie macierzy widoku modelu dla funkcji rysuj�cej
	glMatrixMode(GL_MODELVIEW);
	glLoadIdentity();
}

// Funkcja zegarowa dla animacji (OpenGL):
void ZegarFun(int val) {
	// Odrysowanie sceny:
	glutPostRedisplay();
	// Ponowne wystartowanie zegara:
	glutTimerFunc(1000 / ANIM_FPS, ZegarFun, 0);
}

// Obs�uga klawiatury (OpenGL):
void KeyFun(unsigned char key, int x, int y) {
	if (key == 0x1B)
		exit(0);
	if (key == '+')
		scale += 0.05;
	if (key == '-' && scale > 0.05)
		scale -= 0.05;
	if (key == 'g') {
		PxRigidBodyExt::addLocalForceAtLocalPos(*a_sferaBiala, _force_vec, PxVec3(0, 12, 0), PxForceMode::eIMPULSE);
	}
}

void SpecialKeys(int key, int x, int y)
{
	switch (key)
	{
	case GLUT_KEY_LEFT:
		rotatey -= 4;
		break;
	case GLUT_KEY_UP:
		rotatex -= 4;
		break;
	case GLUT_KEY_RIGHT:
		rotatey += 4;
		break;
	case GLUT_KEY_DOWN:
		rotatex += 4;
		break;
	}

	// odrysowanie okna
	ChangeSize(glutGet(GLUT_WINDOW_WIDTH), glutGet(GLUT_WINDOW_HEIGHT));
}

// obsługa przycisków myszki
void MouseButton(int button, int state, int x, int y)
{
	if (button == GLUT_LEFT_BUTTON)
	{
		glutPostRedisplay();

		button_state = state;
		button1 = button;
		if (state == GLUT_DOWN)
		{
			button_x = x;
			button_y = y;
		}
	}
	if (button == GLUT_RIGHT_BUTTON) {
		glutPostRedisplay();
		if (state == GLUT_DOWN)
		{
			button_x = x;
			button_y = y;

		}
		button1 = button;
		button_state = state;
	}
}
void MouseMotion(int x, int y)
{
	if ((button_state == GLUT_DOWN) && (button1 == GLUT_RIGHT_BUTTON))
	{
		rotatey += 30 * (right - left) / glutGet(GLUT_WINDOW_WIDTH) *(x - button_x);
		button_x = x;
		rotatex -= 30 * (top - bottom) / glutGet(GLUT_WINDOW_HEIGHT) *(button_y - y);
		button_y = y;
		glutPostRedisplay();

	}
	glutPostRedisplay();
}

int main(int argc, char *argv[]) {
	glutInit(&argc, argv);

	// Standardowa inicjalizacja okna GLUT:
	glutInitDisplayMode(GLUT_DOUBLE | GLUT_RGB | GLUT_DEPTH);
	glutInitWindowSize(840, 680);
	glutCreateWindow("Billard");

	// Inicjalizacja OpenGL:
	glEnable(GL_DEPTH_TEST);

	// INICJALIZACJA PhysX:
	init_physx();

	// Ustawienie funkcji zwrotnych GLUT:
	glutDisplayFunc(RenderScene);
	glutReshapeFunc(ChangeSize);
	glutKeyboardFunc(KeyFun);

	// dołączenie funkcji obsługi klawiszy funkcyjnych i klawiszy kursora
	glutSpecialFunc(SpecialKeys);

	// Uruchomienie funkcji zegarowej po raz pierwszy:
	glutTimerFunc(1000 / ANIM_FPS, ZegarFun, 0);

	// obsługa przycisków myszki
	glutMouseFunc(MouseButton);
	// obsługa ruchu kursora myszki
	glutMotionFunc(MouseMotion);
	glutMainLoop();

	// Koniec:
	kill_physx();
	return(0);
}

