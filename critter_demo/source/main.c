#include <nds.h>
#include <math.h>
#include <stdio.h>
#include <stdlib.h>

#ifndef M_PI
#define M_PI 3.14159265358979323846
#endif
static inline int   v16f(float x){ return floattov16(x); }
static inline int   f32f(float x){ return floattof32(x); }
static inline float clampf(float v,float a,float b){ return v<a?a:(v>b?b:v); }
static inline float lerpf(float a,float b,float t){ return a+(b-a)*t; }
static inline float sgn(float x){ return (x>0)-(x<0); }

#define MOVE_SPEED        4.8f
#define AIR_CONTROL       0.60f
#define GRAVITY           16.5f
#define JUMP_VEL          6.4f
#define COYOTE_TIME       0.10f
#define JUMP_BUFFER       0.10f
#define MAX_FALL_SPEED    22.0f
#define FRICTION_GROUND   15.0f
#define FRICTION_AIR      2.5f
#define GLIDE_GRAVITY     4.0f
#define GLIDE_MIN_SPEED   2.0f
#define GLIDE_TURN_DAMP   0.6f
#define TURN_IN_PLACE     10.0f
#define ATTACK_TIME       0.20f
#define ATTACK_RANGE      0.85f
#define HURT_KNOCK        3.5f

#define PW   0.44f
#define PH   1.10f
#define PD   0.44f

#define CAM_DIST          7.2f
#define CAM_HEIGHT        3.3f
#define CAM_YAW_SPEED     0.03f
#define FOV_DEG           70.0f

typedef struct { float x,y,z, sx,sy,sz; } Box;
typedef struct { float x,y,z; int alive; } Orb;

typedef struct {
  float x,y,z;
  float vx,vy,vz;
  float patrolA, patrolB;
  int dir;
  int alive;
  float hurtT;
} BugBot;

typedef struct {
  float x,y,z, sx,sy,sz;
  float phase, speed;
  int axis;
  float amp;
} MPlat;

#define N_PLATS 7
static Box plats[N_PLATS] = {
  {  0, 0,   0,  34, 0.5f, 34},
  {  0, 2,  -6,   6, 0.4f,  2},
  { -4, 3.0f,-11,  3, 0.4f,  3},
  {  4, 4.0f,-16,  3, 0.4f,  3},
  {  0, 4.8f,-21,  4, 0.4f,  2},
  {  0, 6.0f,-26,  6, 0.4f,  6},
  {  0, 7.6f,-33,  8, 0.6f,  8},
};

#define N_MPLAT 2
static MPlat mplat[N_MPLAT] = {
  { -2, 2.8f, -8,  2.0f,0.35f,1.5f,  0, 1.6f,  0, 1.6f },
  {  2, 4.2f,-19,  2.0f,0.35f,1.5f,  0, 1.2f,  1, 1.4f }
};

#define N_ORBS 10
static Orb orbs[N_ORBS];

#define N_BUGS 4
static BugBot bugs[N_BUGS];

static float beaconX=0, beaconY=7.9f, beaconZ=-33.0f;
static int   beaconOn=0;

static struct {
  float x,y,z, vx,vy,vz, yaw;
  int onGround;
  int jumpsLeft;
  float coyoteT, jumpBufT;
  float attackT;
  int   gliding;
  float hp;
  float invulnT;
} P;

static float camYaw=0.0f;

static void drawBox(float sx,float sy,float sz,int r,int g,int b){
  glColor3b(r,g,b);
  glScalef32(f32f(sx), f32f(sy), f32f(sz));
  glBegin(GL_QUADS);
  glVertex3v16(v16f(-0.5f),v16f(-0.5f),v16f( 0.5f));
  glVertex3v16(v16f( 0.5f),v16f(-0.5f),v16f( 0.5f));
  glVertex3v16(v16f( 0.5f),v16f( 0.5f),v16f( 0.5f));
  glVertex3v16(v16f(-0.5f),v16f( 0.5f),v16f( 0.5f));
  glVertex3v16(v16f(-0.5f),v16f(-0.5f),v16f(-0.5f));
  glVertex3v16(v16f( 0.5f),v16f(-0.5f),v16f(-0.5f));
  glVertex3v16(v16f( 0.5f),v16f( 0.5f),v16f(-0.5f));
  glVertex3v16(v16f(-0.5f),v16f( 0.5f),v16f(-0.5f));
  glVertex3v16(v16f( 0.5f),v16f(-0.5f),v16f(-0.5f));
  glVertex3v16(v16f( 0.5f),v16f(-0.5f),v16f( 0.5f));
  glVertex3v16(v16f( 0.5f),v16f( 0.5f),v16f( 0.5f));
  glVertex3v16(v16f( 0.5f),v16f( 0.5f),v16f(-0.5f));
  glVertex3v16(v16f(-0.5f),v16f(-0.5f),v16f(-0.5f));
  glVertex3v16(v16f(-0.5f),v16f(-0.5f),v16f( 0.5f));
  glVertex3v16(v16f(-0.5f),v16f( 0.5f),v16f( 0.5f));
  glVertex3v16(v16f(-0.5f),v16f( 0.5f),v16f(-0.5f));
  glVertex3v16(v16f(-0.5f),v16f( 0.5f),v16f(-0.5f));
  glVertex3v16(v16f( 0.5f),v16f( 0.5f),v16f(-0.5f));
  glVertex3v16(v16f( 0.5f),v16f( 0.5f),v16f( 0.5f));
  glVertex3v16(v16f(-0.5f),v16f( 0.5f),v16f( 0.5f));
  glVertex3v16(v16f(-0.5f),v16f(-0.5f),v16f(-0.5f));
  glVertex3v16(v16f( 0.5f),v16f(-0.5f),v16f(-0.5f));
  glVertex3v16(v16f( 0.5f),v16f(-0.5f),v16f( 0.5f));
  glVertex3v16(v16f(-0.5f),v16f(-0.5f),v16f( 0.5f));
  glEnd();
}

static void drawHero(void){
  glPushMatrix();
    glTranslatef32(f32f(P.x), f32f(P.y + PH*0.5f), f32f(P.z));
    glRotateYi(f32f(P.yaw * 32768.0f / (M_PI*2.0f)));
    drawBox(PW, PH*0.70f, PD, 255,150,60);
    glPushMatrix(); glTranslatef32(0, f32f(PH*0.35f), 0); drawBox(PW*0.9f, PH*0.30f, PD*0.9f, 255,180,90); glPopMatrix(1);
    glPushMatrix(); glTranslatef32(0, f32f(PH*0.65f), 0); drawBox(0.42f,0.38f,0.42f, 255,200,120); glPopMatrix(1);
    glPushMatrix(); glTranslatef32(0, f32f(PH*0.62f), f32f(-0.18f)); drawBox(0.18f,0.14f,0.22f, 240,160,80); glPopMatrix(1);
    glPushMatrix(); glTranslatef32(0, f32f(PH*0.25f), f32f(0.22f)); drawBox(0.10f,0.10f,0.28f, 255,170,80); glPopMatrix(1);
    if(P.attackT > 0.0f){
      float t = P.attackT;
      float ang = (t>ATTACK_TIME*0.4f? 1.0f : t/(ATTACK_TIME*0.4f))*80.0f;
      glPushMatrix();
        glTranslatef32(f32f(0.32f), f32f(PH*0.20f), 0);
        glRotatef32i(f32f(-ang), 0, f32f(0), f32f(1));
        drawBox(0.70f, 0.06f, 0.06f, 230,230,60);
      glPopMatrix(1);
    }
  glPopMatrix(1);
}

static void drawLevel(void){
  for(int i=0;i<N_PLATS;i++){
    glPushMatrix();
      glTranslatef32(f32f(plats[i].x), f32f(plats[i].y), f32f(plats[i].z));
      drawBox(plats[i].sx, plats[i].sy, plats[i].sz, 110,110,120);
    glPopMatrix(1);
  }
  for(int i=0;i<N_MPLAT;i++){
    glPushMatrix();
      glTranslatef32(f32f(mplat[i].x), f32f(mplat[i].y), f32f(mplat[i].z));
      drawBox(mplat[i].sx, mplat[i].sy, mplat[i].sz, 140,140,170);
    glPopMatrix(1);
  }
  glPushMatrix();
    glTranslatef32(f32f(0), f32f(7.9f), f32f(-33.0f));
    drawBox(0.5f,0.7f,0.5f, 80,200,255);
    glPushMatrix(); glTranslatef32(0, f32f(0.6f), 0); drawBox(0.25f,0.25f,0.25f, 0,255,220); glPopMatrix(1);
  glPopMatrix(1);
}

static void drawOrbs(void){
  for(int i=0;i<N_ORBS;i++){
    if(!orbs[i].alive) continue;
    glPushMatrix();
      glTranslatef32(f32f(orbs[i].x), f32f(orbs[i].y), f32f(orbs[i].z));
      drawBox(0.32f,0.32f,0.32f, 0,220,255);
    glPopMatrix(1);
  }
}

static void drawBugs(void){
  for(int i=0;i<N_BUGS;i++){
    if(!bugs[i].alive) continue;
    int pulse = (bugs[i].hurtT>0)? 255 : 180;
    glPushMatrix();
      glTranslatef32(f32f(bugs[i].x), f32f(bugs[i].y+0.25f), f32f(bugs[i].z));
      drawBox(0.7f,0.3f,0.9f, 180,pulse,60);
      glPushMatrix(); glTranslatef32(f32f( 0.25f),0,f32f( 0.35f)); drawBox(0.14f,0.08f,0.18f, 100,80,60); glPopMatrix(1);
      glPushMatrix(); glTranslatef32(f32f(-0.25f),0,f32f( 0.35f)); drawBox(0.14f,0.08f,0.18f, 100,80,60); glPopMatrix(1);
      glPushMatrix(); glTranslatef32(f32f( 0.25f),0,f32f(-0.35f)); drawBox(0.14f,0.08f,0.18f, 100,80,60); glPopMatrix(1);
      glPushMatrix(); glTranslatef32(f32f(-0.25f),0,f32f(-0.35f)); drawBox(0.14f,0.08f,0.18f, 100,80,60); glPopMatrix(1);
    glPopMatrix(1);
  }
}

static int AABBoverlap(Box b, float x,float y,float z, float sx,float sy,float sz){
  return (fabsf(x-b.x)*2.0f < (sx+b.sx)) &&
         (fabsf(y-b.y)*2.0f < (sy+b.sy)) &&
         (fabsf(z-b.z)*2.0f < (sz+b.sz));
}
static Box boxFromPlat(float x,float y,float z,float sx,float sy,float sz){
  Box b; b.x=x; b.y=y; b.z=z; b.sx=sx; b.sy=sy; b.sz=sz; return b;
}

static void resolvePlatforms(float dt){
  P.onGround = 0;
  float px=P.x, py=P.y+PH*0.5f, pz=P.z;
  for(int i=0;i<N_PLATS;i++){
    if(!AABBoverlap(plats[i], px,py,pz, PW,PH,PD)) continue;
    float dx=px-plats[i].x, dy=py-plats[i].y, dz=pz-plats[i].z;
    float ox=(plats[i].sx+PW)*0.5f - fabsf(dx);
    float oy=(plats[i].sy+PH)*0.5f - fabsf(dy);
    float oz=(plats[i].sz+PD)*0.5f - fabsf(dz);
    if(oy<=ox && oy<=oz){
      if(dy>0){ P.y += oy; P.vy=0; P.onGround=1; P.coyoteT=COYOTE_TIME; P.jumpsLeft=1; }
      else{ P.y -= oy; if(P.vy>0) P.vy=0; }
      py=P.y+PH*0.5f;
    }else if(ox<=oz){ P.x += (dx>0? ox:-ox); if(sgn(P.vx)==sgn(dx)) P.vx=0; px=P.x; }
    else{ P.z += (dz>0? oz:-oz); if(sgn(P.vz)==sgn(dz)) P.vz=0; pz=P.z; }
  }
  for(int i=0;i<2;i++){
    Box b = boxFromPlat(mplat[i].x,mplat[i].y,mplat[i].z,mplat[i].sx,mplat[i].sy,mplat[i].sz);
    if(!AABBoverlap(b, px,py,pz, PW,PH,PD)) continue;
    float dx=px-b.x, dy=py-b.y, dz=pz-b.z;
    float ox=(b.sx+PW)*0.5f - fabsf(dx);
    float oy=(b.sy+PH)*0.5f - fabsf(dy);
    float oz=(b.sz+PD)*0.5f - fabsf(dz);
    if(oy<=ox && oy<=oz){
      if(dy>0){ P.y += oy; P.vy=0; P.onGround=1; P.coyoteT=COYOTE_TIME; P.jumpsLeft=1;
        if(mplat[i].axis==0) P.x += (mplat[i].speed*cosf(mplat[i].phase))*0.05f;
        if(mplat[i].axis==1) P.z += (mplat[i].speed*cosf(mplat[i].phase))*0.05f;
      } else { P.y -= oy; if(P.vy>0) P.vy=0; }
      py=P.y+PH*0.5f;
    } else if(ox<=oz){ P.x += (dx>0? ox:-ox); if(sgn(P.vx)==sgn(dx)) P.vx=0; px=P.x; }
      else{ P.z += (dz>0? oz:-oz); if(sgn(P.vz)==sgn(dz)) P.vz=0; pz=P.z; }
  }
}

static void resetOrbs(void){
  float p[N_ORBS][3]={
    {0,1.2f,-2},{0,2.6f,-6},{-3.6f,3.5f,-11},
    {3.6f,4.5f,-16},{0,5.3f,-21}, {-2,6.3f,-26},
    {2,6.3f,-26},{0,7.1f,-29},{-1,7.1f,-31},{1,7.1f,-31}
  };
  for(int i=0;i<N_ORBS;i++){ orbs[i].x=p[i][0]; orbs[i].y=p[i][1]; orbs[i].z=p[i][2]; orbs[i].alive=1; }
}
static void resetBugs(void){
  BugBot bT[N_BUGS]={
    { -6, 1.0f, -4, 0,0,0,  -8, -4, 1, 1, 0,0},
    {  6, 1.0f, -10,0,0,0,   4,  8, -1, 1, 0,0},
    { -2, 2.8f, -8, 0,0,0,  -4,  0, 1, 1, 0,0},
    {  0, 6.0f,-26, 0,0,0,  -2,  2, 1, 1, 0,0}
  };
  for(int i=0;i<N_BUGS;i++){ bugs[i]=bT[i]; }
}
static void resetPlayerToStart(void){
  P.x=0; P.y=1.1f; P.z=2.0f;
  P.vx=P.vy=P.vz=0; P.yaw=0;
  P.onGround=0; P.jumpsLeft=1;
  P.coyoteT=COYOTE_TIME; P.jumpBufT=0; P.attackT=0; P.gliding=0;
  P.hp=3.0f; P.invulnT=0;
  camYaw=0;
  resetOrbs(); resetBugs(); beaconOn=0;
}
static void respawnAtBeacon(void){
  P.x=0; P.y=7.9f+0.6f; P.z=-33.0f+1.2f;
  P.vx=P.vy=P.vz=0; P.yaw=0; P.onGround=0; P.jumpsLeft=1; P.coyoteT=COYOTE_TIME;
  P.invulnT=0.0f;
}

static void updateBugs(float dt){
  for(int i=0;i<N_BUGS;i++){
    if(!bugs[i].alive) continue;
    float speed = 1.4f;
    bugs[i].vx = bugs[i].dir * speed;
    bugs[i].x += bugs[i].vx * dt;

    if(bugs[i].x < bugs[i].patrolA){ bugs[i].x=bugs[i].patrolA; bugs[i].dir=+1; }
    if(bugs[i].x > bugs[i].patrolB){ bugs[i].x=bugs[i].patrolB; bugs[i].dir=-1; }
    if(bugs[i].hurtT>0) bugs[i].hurtT -= dt;

    float dx=P.x-bugs[i].x, dy=(P.y+0.5f)-(bugs[i].y+0.2f), dz=P.z-bugs[i].z;
    float d2=dx*dx+dy*dy+dz*dz;
    if(d2 < 0.7f*0.7f && P.invulnT<=0){
      P.hp -= 1.0f; if(P.hp<0) P.hp=0;
      P.invulnT = 0.9f;
      float ang = atan2f(dz, dx);
      P.vx += cosf(ang)*-HURT_KNOCK; P.vz += sinf(ang)*-HURT_KNOCK; P.vy = 3.0f;
    }

    if(P.attackT>0){
      float fwdx = sinf(P.yaw), fwdz = -cosf(P.yaw);
      float hx = P.x + fwdx*ATTACK_RANGE;
      float hz = P.z + fwdz*ATTACK_RANGE;
      float ddx = hx - bugs[i].x, ddz = hz - bugs[i].z;
      if((ddx*ddx + ddz*ddz) < 0.8f*0.8f){
        bugs[i].alive = 0;
      }
    }
  }
}

static void updatePlatforms(float dt){
  for(int i=0;i<N_MPLAT;i++){
    mplat[i].phase += mplat[i].speed * dt;
    float s = sinf(mplat[i].phase) * mplat[i].amp;
    if(mplat[i].axis==0) mplat[i].x = (i==0? -2: mplat[i].x) + s;
    if(mplat[i].axis==1) mplat[i].z = (i==1? -19: mplat[i].z) + s;
    if(mplat[i].axis==2) mplat[i].y = mplat[i].y + s;
  }
}

static void stepPlayer(float dt){
  scanKeys();
  u16 kH=keysHeld(), kD=keysDown();

  if(kH & KEY_R) camYaw += 0.03f;
  if(kH & KEY_L) camYaw -= 0.03f;
  if(kD & KEY_Y) P.yaw += M_PI;

  float fx = sinf(camYaw), fz = -cosf(camYaw);
  float rx = cosf(camYaw), rz =  sinf(camYaw);

  float ax=0, az=0;
  if(kH & KEY_UP)    { ax += fx; az += fz; }
  if(kH & KEY_DOWN)  { ax -= fx; az -= fz; }
  if(kH & KEY_LEFT)  { ax -= rx; az -= rz; }
  if(kH & KEY_RIGHT) { ax += rx; az += rz; }

  float accel = MOVE_SPEED * (P.onGround? 1.0f : AIR_CONTROL);
  P.vx += ax * accel * dt;
  P.vz += az * accel * dt;

  float targetYaw = P.yaw;
  if(ax!=0 || az!=0){ targetYaw = atan2f(P.vx, -P.vz); }
  float turnRate = (P.onGround? 10.0f : 6.0f);
  float diff = targetYaw - P.yaw;
  while(diff >  M_PI) diff-=2*M_PI;
  while(diff < -M_PI) diff+=2*M_PI;
  P.yaw += clampf(diff, -turnRate*dt, turnRate*dt);

  if(kD & KEY_A) P.jumpBufT = 0.10f;
  if(P.coyoteT > 0 && P.jumpBufT > 0){
    P.vy = 6.4f; P.onGround=0; P.coyoteT=0; P.jumpBufT=0;
  }

  P.gliding = 0;
  if(!(P.onGround) && (kH & KEY_A)){
    P.gliding = 1;
    P.vy -= 4.0f * dt;
    float fwd = fx*P.vx + fz*P.vz;
    if(fwd < 2.0f){
      P.vx += fx * (2.0f - fwd) * 0.5f;
      P.vz += fz * (2.0f - fwd) * 0.5f;
    }
    float side = rx*P.vx + rz*P.vz;
    P.vx -= rx*side*(1.0f-0.6f)*dt*10.0f;
    P.vz -= rz*side*(1.0f-0.6f)*dt*10.0f;
  }

  if(kD & KEY_B) P.attackT = 0.20f;

  float fr = (P.onGround? 15.0f : 2.5f);
  P.vx -= P.vx * fr * dt;
  P.vz -= P.vz * fr * dt;

  if(!P.gliding) P.vy -= 16.5f * dt;
  if(P.vy < -22.0f) P.vy = -22.0f;

  P.x += P.vx * dt;
  P.y += P.vy * dt;
  P.z += P.vz * dt;

  if(P.onGround) P.coyoteT = 0.10f; else P.coyoteT -= dt;
  P.jumpBufT -= dt;
  if(P.attackT>0) P.attackT -= dt;
  if(P.invulnT>0) P.invulnT -= dt;

  resolvePlatforms(dt);

  if(P.y < -4.0f){
    if(beaconOn) { P.x=0; P.y=7.9f+0.6f; P.z=-33.0f+1.2f; }
    else resetPlayerToStart();
    return;
  }

  for(int i=0;i<N_ORBS;i++){
    if(!orbs[i].alive) continue;
    float dx=P.x-orbs[i].x, dy=(P.y+0.6f)-orbs[i].y, dz=P.z-orbs[i].z;
    if(dx*dx+dy*dy+dz*dz < 0.55f*0.55f) orbs[i].alive=0;
  }

  float bx=P.x-0, by=(P.y+0.6f)-7.9f, bz=P.z-(-33.0f);
  if(bx*bx+by*by+bz*bz < 1.0f*1.0f) beaconOn=1;
}

static void applyCamera(void){
  float camX = P.x - sinf(camYaw)*7.2f;
  float camZ = P.z + cosf(camYaw)*7.2f;
  float camY = P.y + 3.3f;

  glMatrixMode(GL_MODELVIEW);
  glLoadIdentity();
  gluLookAt(
    camX, camY, camZ,
    P.x, P.y + 0.6f, P.z,
    0,1,0
  );
}

static void hud(void){
  consoleClear();
  int collected=0; for(int i=0;i<N_ORBS;i++) if(!orbs[i].alive) collected++;
  iprintf("\\x1b[02;02H3D Critter Rooftops   ");
  iprintf("\\x1b[04;02HOrbs: %d/%d", collected, N_ORBS);
  iprintf("\\x1b[06;02HHP: %.0f   Glide(A)  Swat(B)  Cam(L/R)", P.hp);
  if(beaconOn) iprintf("\\x1b[08;02HCheckpoint active");
}

static void videoInit3D(void){
  videoSetMode(MODE_0_3D);
  videoSetModeSub(MODE_0_2D);
  vramSetBankA(VRAM_A_TEXTURE);
  vramSetBankB(VRAM_B_TEXTURE);
  vramSetBankC(VRAM_C_SUB_BG);
  vramSetBankD(VRAM_D_MAIN_BG);
  consoleDemoInit();

  glInit();
  glEnable(GL_ANTIALIAS);
  glClearColor(6,6,10, 31);
  glClearPolyID(63);
  glClearDepth(0x7FFF);

  glLight(0, RGB15(31,31,31), 0, 1<<12, 1<<12);
  glMaterialf(GL_AMBIENT, RGB15(8,8,8));
  glMaterialf(GL_DIFFUSE, RGB15(28,28,28));

  glMatrixMode(GL_PROJECTION);
  glLoadIdentity();
  gluPerspectivef32(f32f(70.0f), f32f(256.0f/192.0f), f32f(0.1f), f32f(120.0f));
}

int main(void){
  videoInit3D();
  resetPlayerToStart();
  mplat[0].phase = 0.0f;
  mplat[1].phase = 1.2f;

  while(1){
    swiWaitForVBlank();
    float dt = 1.0f/60.0f;

    for(int i=0;i<2;i++){
      mplat[i].phase += mplat[i].speed * dt;
      float s = sinf(mplat[i].phase) * mplat[i].amp;
      if(mplat[i].axis==0) mplat[i].x = -2 + s;
      if(mplat[i].axis==1) mplat[i].z = -19 + s;
    }

    scanKeys();
    stepPlayer(dt);

    for(int i=0;i<N_BUGS;i++){
      if(!bugs[i].alive) continue;
      float speed = 1.4f;
      bugs[i].vx = bugs[i].dir * speed;
      bugs[i].x += bugs[i].vx * dt;
      if(bugs[i].x < bugs[i].patrolA){ bugs[i].x=bugs[i].patrolA; bugs[i].dir=+1; }
      if(bugs[i].x > bugs[i].patrolB){ bugs[i].x=bugs[i].patrolB; bugs[i].dir=-1; }
      if(bugs[i].hurtT>0) bugs[i].hurtT -= dt;
    }

    glMatrixMode(GL_MODELVIEW);
    applyCamera();

    drawLevel();
    drawOrbs();
    drawBugs();
    drawHero();

    glFlush(0);
    hud();
  }
  return 0;
}
