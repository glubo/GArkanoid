//toto je arkanoid napsany Glubem k prilezitosti 29hod souteze na ceskehry.cz dne 22.-23.7.2004
//kod velmi prasacky a bez komentaru, ale nepredpokladam, ze by ho nekdo cetl
//psano ve vimu


//par konstant
#define NUM_COLS 14
#define NUM_ROWS 14
#define ROW_HEIGHT 16
#define COL_WIDTH  32
#define BOTTOM (NUM_ROWS+2)*ROW_HEIGHT
#define STICK_WIDTH 32
#define STICK_HEIGHT 8
#define BALL_SPEED 250.0f
#define NUM_LEVELS 9

#include <SDL.h>
#include <SDL_timer.h>
#include <SDL_mixer.h>
#include <SDL_opengl.h>
#include <SDL_image.h>
#include <stdio.h>
#include <vector>
#include <list>
#include <math.h>

#define RECT(x,y,width,height)	glBegin(GL_TRIANGLE_STRIP);glVertex2f(x, y+1*height);glVertex2f(x, y);glVertex2f(x+1*width, y+1*height);glVertex2f(x+1*width, y);glEnd();
#define TRECT(x,y,width,height)	glBegin(GL_TRIANGLE_STRIP);glTexCoord2f(0.0,1.0);glVertex2f(x, y+1*height);glTexCoord2f(0.0,0.0);glVertex2f(x, y);glTexCoord2f(1.0,1.0);glVertex2f(x+1*width, y+1*height);glTexCoord2f(1.0,0.0);glVertex2f(x+1*width, y);glEnd();

class Object;
class ObjectUpdater;
class Bricks;
class Level;
class Arkanoid;
class Bonus;
class ParticleEngine;
//--------------------------------------------------------------------------------------
typedef enum BRICK_TYPE
{
	BRICK_NONE=0,
	BRICK_NORMAL=1,
	BRICK_HARD=2,
	BRICK_NONBREAKABLE=3
};
//--------------------------------------------------------------------------------------
typedef enum BONUS_TYPE{
	SIZE,
	SLOW,
	SCORE,
	BIGSCORE,
	SUPERBALL,
	LEVEL,
	NO_BTYPES
};
//--------------------------------------------------------------------------------------
typedef struct Brick
{
	BRICK_TYPE type;	
};
//--------------------------------------------------------------------------------------
template<class T> class tvec3
{
protected:
	T X, Y, Z;
public:
	tvec3(T x, T y, T z):X(x),Y(y),Z(z){};
	inline T x(){return X;};
	inline T y(){return Y;};
	inline T z(){return Z;};

	inline tvec3 operator *(const T &b){return tvec3( X*b, Y*b, Z*b);};
	inline tvec3 operator /(const T &b){return tvec3( X/b, Y/b, Z/b);};
	inline tvec3 operator *=(const T &b){X*=b; Y*=b; Z*=b;return *this;};
	inline tvec3 operator /=(const T &b){X/=b; Y/=b; Z/=b;return *this;};
	inline T operator *(const tvec3 &b){return ( X*b.X + Y*b.Y + Z*b.Z);};
//typedef double gfloat;
#define gfloat float
	inline tvec3 operator +(const tvec3 &b){return tvec3( X+b.X, Y+b.Y, Z+b.Z);};
	inline tvec3 operator -(const tvec3 &b){return tvec3( X-b.X, Y-b.Y, Z-b.Z);};
	inline tvec3 operator +=(const tvec3 &b){X+=b.X; Y+=b.Y; Z+=b.Z;return *this;};

	inline T SLen(){return (*this)*(*this);};
	inline T Len(){return sqrt(SLen());};
	inline void Normalize(){T l=Len();X/=l;Y/=l;Z/=l;};
};
//--------------------------------------------------------------------------------------
template<class T>class tvec2
{
protected:
	T X, Y;
public:
	tvec2(T x=0, T y=0):X(x),Y(y){};
	inline T x(){return X;};
	inline T y(){return Y;};
	inline void x(T nx){X=nx;};
	inline void y(T ny){Y=ny;};

	inline tvec2 operator *(const T &b){return tvec2( X*b, Y*b);};
	inline tvec2 operator /(const T &b){return tvec2( X/b, Y/b);};
	inline tvec2 operator *=(const T &b){X*=b; Y*=b;return *this;};
	inline tvec2 operator /=(const T &b){X/=b; Y/=b;return *this;};
	inline T operator *(const tvec2 &b){return ( X*b.X + Y*b.Y);};
	inline tvec2 operator +(const tvec2 &b){return tvec2( X+b.X, Y+b.Y);};
	inline tvec2 operator -(const tvec2 &b){return tvec2( X-b.X, Y-b.Y);};
	inline tvec2 operator +=(const tvec2 &b){X+=b.X; Y+=b.Y;return *this;};
	inline tvec2 operator -=(const tvec2 &b){X-=b.X; Y-=b.Y;return *this;};
	inline T SLen(){return (*this)*(*this);};
	inline T Len(){return sqrt(SLen());};
	inline void Normalize(){T l=Len();X/=l;Y/=l;};
	inline bool operator !=(const tvec2 &b){return ( X!=b.X || Y!=b.Y );};
	inline bool operator >(const tvec2 &b){return ( X>b.X && Y>b.Y );};
	inline bool operator <(const tvec2 &b){return ( X<b.X && Y<b.Y );};
};
//--------------------------------------------------------------------------------------
typedef tvec2<float> vec2;
typedef tvec2<int> ivec2;
typedef tvec3<float> vec3;

inline ivec2 B2B(vec2 ball);

//--------------------------------------------------------------------------------------
ivec2 B2B(vec2 ball)
{
	return ivec2((int)ball.x()/COL_WIDTH, (int)ball.y()/ROW_HEIGHT);
};
//--------------------------------------------------------------------------------------
class Arkanoid
{
protected:
	Level *alevel;
	Bonus *bonus;
	ParticleEngine *pe;
	int score;
	int lscore;
	bool game;
	bool exit;
  	Uint8 *keys;
	bool done;
	int lives;
	int llives;
	int level;

	SDL_Surface *brick;
	GLuint	    tbrick;
	GLuint	    tstick;
	GLuint	    tball;
	GLuint	    tfont;
	GLuint	    base;
	GLuint	    twall;
	GLuint	    tbonus;
	
	Mix_Chunk *sstick,*sbounce,*sbrick,*snbreak;                  // sfx
	Mix_Music *mmusic;
public:
	Arkanoid();
	~Arkanoid();


	void DrawBrick(const Brick &b, int x, int y);
	void DrawParticle(vec2 pos);
	void DrawBonus(BONUS_TYPE, vec2 pos);
	void DrawBall(vec2 &pos,bool super);
	void DrawStick(vec2 &pos, vec2 &size);
	void DeathBrick(int x, int y);
	void CrackBrick(int x, int y);
	void Death();
	void Side();
	void Stick();
	float GetStickVel();
	void Game();
	void BuildFont();
	void KillFont();
	void NextLevel();

protected:
	void glPrint(GLint x,GLint y,int set, const char *text);
	void Redraw();
	void Update(float dtime);
	void InitGL(int Width, int Height);
	void LoadTextures();
	void ReleaseTextures();
};
Arkanoid * gArkanoid;
//--------------------------------------------------------------------------------------
class Bricks
{
protected:
	Brick b[NUM_COLS][NUM_ROWS];
	
public:
	Bricks();
	~Bricks();
	
	void Redraw();
	void Update(float dtime){};

	bool Cleared();
	bool Collide(vec2 pos);	//vrati true pokud micek narazil, postara se o odstraneni bricku

	bool Load(char *fn);	
};

Bricks::Bricks()
{
	int px,py;
	for(px=0;px<NUM_COLS;px++)
		for(py=0;py<NUM_ROWS;py++)
			b[px][py].type=BRICK_NONE;
};

Bricks::~Bricks()
{
};

void Bricks::Redraw()
{
	int px,py;

	for(px=0;px<NUM_COLS;px++)
		for(py=0;py<NUM_ROWS;py++)
			gArkanoid->DrawBrick(b[px][py],px,py);
};

bool Bricks::Cleared()
{
	int px,py;

	for(px=0;px<NUM_COLS;px++)
		for(py=0;py<NUM_ROWS;py++)
			if( b[px][py].type!=BRICK_NONE && b[px][py].type!=BRICK_NONBREAKABLE)
			{
				printf("jesne necleared na %d %d\n", px, py);
				return false;
			};
	printf("Cleared\n");
	return true;
};

bool Bricks::Collide(vec2 pos)
{
	ivec2 ppos;

	ppos=B2B(pos);
	if( ppos.x()<0 || ppos.x()>=NUM_COLS ) return false;
	if( ppos.y()<0 || ppos.y()>=NUM_ROWS ) return false;
	
	switch(b[ppos.x()][ppos.y()].type)
	{
		case BRICK_NONE: return false;
		case BRICK_NONBREAKABLE: printf("Collision NONBREAK %d %d\n",ppos.x(),ppos.y());gArkanoid->Side(); return true;
		case BRICK_NORMAL:printf("Collision BREAK %d %d\n",ppos.x(),ppos.y());b[ppos.x()][ppos.y()].type=BRICK_NONE; gArkanoid->DeathBrick(ppos.x(),ppos.y());return true;
		case BRICK_HARD:printf("Collision HBREAK %d %d\n",ppos.x(),ppos.y()); gArkanoid->CrackBrick(ppos.x(),ppos.y());b[ppos.x()][ppos.y()].type=BRICK_NORMAL;return true;
	};
	return false;
};

bool Bricks::Load(char *fn)
{
	int px,py,ppom;
	FILE *fp;

	if(!(fp=fopen(fn,"r"))){fprintf(stderr,"Can't load %s\n",fn);return false;};
	for(py=0;py<NUM_ROWS;py++)
		for(px=0;px<NUM_COLS;px++)
		{
			if(feof(fp)){fprintf(stderr,"Unexpected end of file in %s\n",fn);fclose(fp);return false;};
			fscanf(fp,"%d",&ppom);
			b[px][py].type=(BRICK_TYPE)ppom;
		};
	fclose(fp);
	return true;
};
//--------------------------------------------------------------------------------------
class Ball
{
public:
	vec2 pos;
	vec2 lpos;
	ivec2 lcell;
	vec2 vel;
	bool super;
	float srem;
public:
	Ball(){super=false;};
	~Ball(){};
	
	const vec2 Pos(){return pos;};
	void Pos(vec2 npos){pos=npos;};
	void Vel(vec2 nvel){vel=nvel;};
	void Slow(){vel.Normalize();vel*=BALL_SPEED;};
	void Super(){super=true;srem=10;};
	void CollidedWithLSide(){
		vel=vec2(fabs(vel.x()),vel.y());
		printf("Collided With Side\n");
		gArkanoid->Side();
	};
	void CollidedWithRSide(){
		vel=vec2(0.0-fabs(vel.x()),vel.y());
		printf("Collided With Side\n");
		gArkanoid->Side();
	};
	void CollidedWithTop(){
		vel=vec2(vel.x(),fabs(vel.y())+0.01f);
		printf("Collided With Top\n");
		gArkanoid->Side();
	};
	void Update(float dtime){
		pos+=vel*dtime;
		if( B2B(pos)!=lcell )lcell=B2B(lpos);
		lpos=pos;
		if(pos.y()<=0.0)CollidedWithTop();
		if(pos.y()>=BOTTOM)gArkanoid->Death();

		if(pos.x()<=0.0)CollidedWithLSide();
		if(pos.x()>=NUM_COLS*COL_WIDTH)CollidedWithRSide();
		if(super){
			srem-=dtime;
			if(srem<=0)super=false;
		};
	};
	void Redraw(){gArkanoid->DrawBall(pos,super);};
	void CollidedWithBrick(){
		vel*=1.005;
		ivec2 rpos;
		if(!super)
		{
			rpos=B2B(pos);
			rpos-=lcell;
			if(rpos.x()!=0)
			{
				vel.x( 0.0-vel.x() );
			};
			if(rpos.y()!=0)
			{
				vel.y( 0.0-vel.y() );
			};
		
			pos=lpos;
		};
		printf("Collided With Block\n");
	};
	void CollidedWithStick(vec2 where){
		vec2 pvec;
		pvec=pos-where-vec2(0,20);
		vel=pvec*vel.Len()/pvec.Len();	

		printf("Collided With Stick\n");
		gArkanoid->Stick();
	};
		
};
//--------------------------------------------------------------------------------------
class Stick{
protected:
	vec2 pos;
	vec2 size;
public:
	Stick(){};
	~Stick(){};

	void Pos(vec2 const npos){pos=npos;};
	void Size(vec2 const nsize){size=nsize;};
	vec2 Pos(){return pos;};
	vec2 Size(){return size;};
	void Update(float dtime);//definice pod Arkanoid
	void Redraw(){gArkanoid->DrawStick(pos,size);};
	bool Collide(vec2 ball){
		//if( (pos+size)<ball && (pos-size)>ball)
		
		if( (pos.x()+size.x())>ball.x() && (pos.x()-size.x())<ball.x())
			if( (pos.y()+size.y())>ball.y() && (pos.y()-size.y())<ball.y())
				{printf("Stick->Collide()true\n");return true;};
		return false;
	};
};
//--------------------------------------------------------------------------------------
class Level
{
public:
	Bricks	bricks;
	Ball	ball;
	Stick	stick;
	bool	stuckball;
public:
	Level(){};
	~Level(){};

	bool NewLevel(char *fn)
	{
		if(!bricks.Load(fn))return false;
		stick.Size( vec2(STICK_WIDTH, STICK_HEIGHT));
		stick.Pos( vec2( NUM_COLS*COL_WIDTH/2, BOTTOM - STICK_HEIGHT/2) );
		ball.Pos( stick.Pos()-vec2(0,STICK_HEIGHT/2));
		ball.Slow();
		stuckball=true;

		return true;
	};
	
	void Update(float dtime){
		bricks.Update(dtime);
		stick.Update(dtime);
		if(stuckball)
		{
			ball.Pos( stick.Pos()-vec2(0,STICK_HEIGHT));
		}else{
			ball.Update(dtime);
			if(bricks.Collide(ball.Pos()))
				ball.CollidedWithBrick();
			if(stick.Collide(ball.Pos()))
				ball.CollidedWithStick(stick.Pos());
		};
	};	
	void Redraw()
	{
		bricks.Redraw();
		stick.Redraw();
		ball.Redraw();
	};	
	void UnStuckBall()
	{
		vec2 pvec;
		if(stuckball)
		{
			stuckball=false;
			pvec=vec2(rand()%100-50,0-(rand()%90+10));
			pvec.Normalize();
			ball.Vel(pvec*BALL_SPEED);
		};
	};
	void StuckBall()
	{
		stuckball=true;
	};
	
	bool Cleared(){return bricks.Cleared();};
};
//--------------------------------------------------------------------------------------
class Bonus
{
	vec2 pos;
	BONUS_TYPE type;
	bool alive;
public:
	Bonus(){alive=false;};
	~Bonus(){};

	void AddBonus(vec2 where){
		if(!alive)
			if(rand()%100<=20)
			{
				alive=true;
				type=(BONUS_TYPE)(rand()%NO_BTYPES);
				pos=where;
			};
	};

	BONUS_TYPE Collide(vec2 pstick, vec2 size)
	{
		if(alive)if( (pstick+size)>pos && (pstick-size)<pos ) 
		{
			alive=false;
			return type;
		};
		return NO_BTYPES;
	};

	void Update(float dtime){
		if(alive){
			pos.y(pos.y()+dtime*100.0f);
			if(pos.y()> BOTTOM +10)
				alive=false;
		};
	};

	void Redraw()
	{
		if(alive){
			gArkanoid->DrawBonus(type,pos);
		};
	};

};
//--------------------------------------------------------------------------------------
#define G 500.0
class Particle{
protected:
	vec2 pos;
	vec2 vel;
	bool alive;
public:
	Particle(vec2 pos=vec2(0,BOTTOM), vec2 vel=vec2(0,0)):pos(pos),vel(vel){alive=true;};
	~Particle(){};

	void Update(float dtime){
		if(alive)
		{
			pos+=vel*dtime;
			vel+=vec2(0,G)*dtime;
			if(pos.y()>BOTTOM)
				alive=false;
		};
	};
	void Redraw(){
		if(alive)
			gArkanoid->DrawParticle(pos);
	};
	friend class ParticleEngine;
};

#define MAX_PARTS 20
class ParticleEngine{
protected:
	Particle parts[MAX_PARTS];
public:
	ParticleEngine(){};
	~ParticleEngine(){};

	void Update(float dtime){
		int ppom;
		for(ppom=0;ppom<MAX_PARTS;ppom++){
			parts[ppom].Update(dtime);
		};
	};

	void Redraw(){
		int ppom;
		for(ppom=0;ppom<MAX_PARTS;ppom++){
			parts[ppom].Redraw();
		};
	};

	void AddPart(vec2 pos){
		int ppom;
		for(ppom=0;ppom<MAX_PARTS;ppom++){
			if(!(parts[ppom].alive))
			{
				parts[ppom]=Particle(pos, vec2(rand()%50-25, 0-rand()%50));
				break;
			};
		};
	};
		
	
};
//--------------------------------------------------------------------------------------
void Arkanoid::Update(float dtime){
	if(game)alevel->Update(dtime);
	if(keys[SDLK_SPACE])
		alevel->UnStuckBall();
	bonus->Update(dtime);
	switch(bonus->Collide(alevel->stick.Pos(),alevel->stick.Size()))
	{
		case NO_BTYPES:break;
		case SIZE:alevel->stick.Size(alevel->stick.Size()*1.5);break;
		case SLOW:alevel->ball.Slow();break;
		case SCORE:score+=100;break;
		case SUPERBALL:alevel->ball.Super();break;
 		case BIGSCORE:score+=500;break;
		case LEVEL:NextLevel();break;
	};

	if( (lscore!=score) || (llives!=lives) )
	{
		char caption[256];
		sprintf(caption,"Garkanoid: Score:%d Lives:%d Level:%d",score,lives,level);
		SDL_WM_SetCaption(caption,NULL);
	};
	lscore=score;
	llives=lives;
	pe->Update(dtime);
};

void Arkanoid::Redraw(){
	glClear(GL_COLOR_BUFFER_BIT);//GR_replace
//	glBindTexture(GL_TEXTURE_2D,twall);
//	glEnable(GL_TEXTURE_2D);
//	TRECT(0,0,640,480);	
//	glDisable(GL_TEXTURE_2D);
	pe->Redraw();
	bonus->Redraw();
	alevel->Redraw();
	
};
Mix_Chunk * load_sound(char * file)                     // Load a sound file
{
  Mix_Chunk * sound;
  sound = Mix_LoadWAV(file);
  if (!sound)
  {
    fprintf(stderr, "Error: %s\n\n", SDL_GetError());
    SDL_Quit();
    exit(2);
  }
  return(sound);
}

Uint32 callback_fps(Uint32 interval, void *param)
{
        SDL_Event event;
        event.type = SDL_USEREVENT;
        SDL_PushEvent(&event);

        return 1000;
}

Arkanoid::Arkanoid(){

	alevel=new Level();
	bonus=new Bonus();
	pe=new ParticleEngine();
	done=false;
	lives=3;
	level=1;
	game=true;
	

  	if ( SDL_Init(SDL_INIT_TIMER|SDL_INIT_VIDEO|SDL_INIT_AUDIO) < 0 ) {
    		fprintf(stderr, "Unable to initialize SDL: %s\n", SDL_GetError());
    		return;
  	}
	
       SDL_GL_SetAttribute( SDL_GL_DOUBLEBUFFER, 1 );//GR_replace
	
	if ( SDL_SetVideoMode(COL_WIDTH*NUM_COLS, 480, 0, SDL_OPENGL) == NULL ) {//GR_replace
	    fprintf(stderr, "Unable to create OpenGL screen: %s\n", SDL_GetError());//GR_replace
	    SDL_Quit();//GR_replace
	    return;//GR_replace
	}

	SDL_WM_SetCaption("Garkanoid", NULL);

  	InitGL(COL_WIDTH*NUM_COLS, 480);//GR_replace
	if(!alevel->NewLevel("lev1.txt")){printf("level not found\n");done=true;};
  if (Mix_OpenAudio(22050, AUDIO_S16, 2, 512) < 0)
  {
    fprintf(stderr, "Error: %s\n\n", SDL_GetError());
  }

  sbrick= load_sound("brick.wav");                 // Load Sounds
  sbounce = load_sound("bounce.wav");
  snbreak = load_sound("nbreak.wav");
  sstick = load_sound("stick.wav");

  mmusic= Mix_LoadMUS("music.mid");
  Mix_PlayMusic(mmusic,-1);
	SDL_AddTimer(1000, callback_fps, NULL);
};

void Arkanoid::Game()
{
	float ltime,atime;
	int frames=0;
	
	ltime=atime=SDL_GetTicks();
	while ( ! done ) {

		atime=SDL_GetTicks();

  		keys = SDL_GetKeyState(NULL);
		

		Update(0.001*(atime-ltime));
		if(!game)done=1;
		SDL_GL_SwapBuffers();//GR_replace
		Redraw();
		frames++;
		ltime=atime;
		SDL_Delay(1);  //This dissmited the bug with >100ms running time of SDL_GL_SwapBuffers ---FUCKING SDL
		if(frames>50){
			SDL_Delay(10);
		}
		
    /* This could go in a separate function */
    { SDL_Event event;
      while ( SDL_PollEvent(&event) ) {
        if ( event.type == SDL_QUIT ) {
            Mix_HaltChannel(-1);
            Mix_CloseAudio();
	    printf("Got SDL_QUIT\n");
          done = 1;
        }
        if ( event.type == SDL_KEYDOWN ) {
          if ( event.key.keysym.sym == SDLK_ESCAPE ) {

		printf("Got ESCAPE\n");
            Mix_HaltChannel(-1);
            Mix_CloseAudio();
            done = 1;
          }
        }
			if(event.type == SDL_USEREVENT){
				printf("%d frames\n",frames);
				frames=0;
			}
      }
    }
  }
            Mix_HaltChannel(-1);
            Mix_CloseAudio();
};


Arkanoid::~Arkanoid(){
  SDL_Quit();
	delete alevel;
	delete bonus;
	delete pe;
};

void Arkanoid::DrawParticle(vec2 pos){
	glColor3ub(200,200,200);//GR_replace
	
	glBindTexture(GL_TEXTURE_2D,tbrick);//GR_replace
	glEnable(GL_TEXTURE_2D);//GR_replace
		TRECT(pos.x(), pos.y(), COL_WIDTH, ROW_HEIGHT);//GR_replace
	glDisable(GL_TEXTURE_2D);//GR_replace
	
};
void Arkanoid::DrawBrick(const Brick &b, int x, int y){
	switch(b.type)
	{
		case BRICK_NONE: return;
		case BRICK_NORMAL:
			glColor3ub(155,0,0);break;//GR_replace
		case BRICK_HARD:
			glColor3ub(100,50,50);break;//GR_replace
		case BRICK_NONBREAKABLE:
			glColor3ub(100,100,100);break;//GR_replace
	};
	glBindTexture(GL_TEXTURE_2D,tbrick);//GR_replace
	glEnable(GL_TEXTURE_2D);//GR_replace
		TRECT(x*COL_WIDTH, y*ROW_HEIGHT, COL_WIDTH, ROW_HEIGHT);//GR_replace
	glDisable(GL_TEXTURE_2D);//GR_replace
	
};

void Arkanoid::DrawBonus(BONUS_TYPE type, vec2 pos){
	glBindTexture(GL_TEXTURE_2D,tbonus);//GR_replace
	
	switch(type)
	{
		case SIZE:glColor3ub(100,100,0);break;//GR_replace
		case SLOW:glColor3ub(0,200,0);break;//GR_replace
		case SCORE:glColor3ub(100,0,100);break;//GR_replace
		case SUPERBALL:glColor3ub(255,0,0);break;//GR_replace
		case BIGSCORE:glColor3ub(200,0,200);break;//GR_replace
		case LEVEL:glColor3ub(0,0,255);break;//GR_replace
	};
	glEnable(GL_TEXTURE_2D);//GR_replace
	TRECT(pos.x(),pos.y(),16,16);//GR_replace
	glDisable(GL_TEXTURE_2D);//GR_replace
};

void Arkanoid::DrawBall(vec2 &pos,bool super){
	glColor3ub(255,255,255);//GR_replace
	if(super)glColor3ub(255,0,0);//GR_replace
	glBindTexture(GL_TEXTURE_2D,tball);//GR_replace
	glEnable(GL_TEXTURE_2D);//GR_replace
	TRECT(pos.x(),pos.y(),6,6);//GR_replace
	glDisable(GL_TEXTURE_2D);//GR_replace
};

void Arkanoid::DrawStick(vec2 &pos,vec2 &size){
	//glColor3ub(55,255,255);
	glColor3ub(255,255,255);//GR_replace
	glBindTexture(GL_TEXTURE_2D,tstick);//GR_replace
	glEnable(GL_TEXTURE_2D);//GR_replace
	TRECT(pos.x()-size.x(),pos.y()-size.y(),size.x()*2,size.y()*2);//GR_replace
	glDisable(GL_TEXTURE_2D);//GR_replace
};

void Arkanoid::DeathBrick(int x, int y){
	if(alevel->Cleared())
	{
		NextLevel();
	};
	bonus->AddBonus(vec2(COL_WIDTH*x,ROW_HEIGHT*y));
	Mix_PlayChannel(-1, sbrick, 0);
	score+=20;
	pe->AddPart(vec2(x*COL_WIDTH, y*ROW_HEIGHT));
};

void Arkanoid::CrackBrick(int x, int y){
	Mix_PlayChannel(-1, snbreak, 0);
	score+=25;
};

void Arkanoid::Stick(){
	Mix_PlayChannel(-1, sstick, 0);
};

void Arkanoid::Side(){
	Mix_PlayChannel(-1, sstick, 0);
};

void Arkanoid::Death(){
	lives--;
	if(lives)
		alevel->StuckBall();
	else game=false;
		
};

float Arkanoid::GetStickVel(){
	if(keys[SDLK_LEFT])return -500.0f;
	if(keys[SDLK_RIGHT])return 500.0f;
	
	return 0.0f;
};


void Arkanoid::InitGL(int Width, int Height)//GR_replace
{
    LoadTextures();
//    glShadeModel(GL_SMOOTH);

    glClearColor(0.0f, 0.0f, 0.0f, 0.5f);
    glClearDepth(1.0);
    glHint(GL_LINE_SMOOTH_HINT, GL_NICEST);
    //glEnable(GL_BLEND);
    //glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

    glViewport(0, 0, Width, Height);
    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();

    glOrtho(0.0f,Width,Height,0.0f,-1.0f,1.0f);

    glMatrixMode(GL_MODELVIEW);
    glLoadIdentity();
}

void Arkanoid::LoadTextures()
{
	brick=IMG_Load("brick.bmp");//GR_replace

    // Create Texture
    glGenTextures(1, &tbrick);//GR_replace
    glBindTexture(GL_TEXTURE_2D, tbrick);   // 2d texture (x and y size)//GR_replace
//GR_replace
    glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_MAG_FILTER,GL_LINEAR); // scale linearly when image bigger than texture//GR_replace
    glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_MIN_FILTER,GL_LINEAR); // scale linearly when image smalled than texture//GR_replace

    // 2d texture, level of detail 0 (normal), 3 components (red, green, blue), x size from image, y size from image,
    // border 0 (normal), rgb color data, unsigned byte data, and finally the data itself.
    glTexImage2D(GL_TEXTURE_2D, 0, 3, brick->w, brick->h, 0, GL_RGB, GL_UNSIGNED_BYTE, brick->pixels);//GR_replace

    brick=IMG_Load("ball.bmp");//GR_replace

    // Create Texture
    glGenTextures(1, &tball);//GR_replace
    glBindTexture(GL_TEXTURE_2D, tball);   // 2d texture (x and y size)//GR_replace

    glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_MAG_FILTER,GL_LINEAR); // scale linearly when image bigger than texture//GR_replace
    glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_MIN_FILTER,GL_LINEAR); // scale linearly when image smalled than texture//GR_replace

    // 2d texture, level of detail 0 (normal), 3 components (red, green, blue), x size from image, y size from image,//GR_replace
    // border 0 (normal), rgb color data, unsigned byte data, and finally the data itself.//GR_replace
    glTexImage2D(GL_TEXTURE_2D, 0, 3, brick->w, brick->h, 0, GL_RGB, GL_UNSIGNED_BYTE, brick->pixels);//GR_replace

    brick=IMG_Load("bonus.bmp");//GR_replace

    // Create Texture
    glGenTextures(1, &tbonus);//GR_replace
    glBindTexture(GL_TEXTURE_2D, tbonus);   // 2d texture (x and y size)//GR_replace

    glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_MAG_FILTER,GL_LINEAR); // scale linearly when image bigger than texture//GR_replace
    glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_MIN_FILTER,GL_LINEAR); // scale linearly when image smalled than texture//GR_replace

    // 2d texture, level of detail 0 (normal), 3 components (red, green, blue), x size from image, y size from image,//GR_replace
    // border 0 (normal), rgb color data, unsigned byte data, and finally the data itself.//GR_replace
    glTexImage2D(GL_TEXTURE_2D, 0, 3, brick->w, brick->h, 0, GL_BGR, GL_UNSIGNED_BYTE, brick->pixels);//GR_replace

 
    brick=IMG_Load("stick.bmp");//GR_replace

    // Create Texture//GR_replace
    glGenTextures(1, &tstick);//GR_replace
    glBindTexture(GL_TEXTURE_2D, tstick);   // 2d texture (x and y size)//GR_replace

    glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_MAG_FILTER,GL_LINEAR); // scale linearly when image bigger than texture//GR_replace
    glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_MIN_FILTER,GL_LINEAR); // scale linearly when image smalled than texture//GR_replace
//GR_replace
    // 2d texture, level of detail 0 (normal), 3 components (red, green, blue), x size from image, y size from image,//GR_replace
    // border 0 (normal), rgb color data, unsigned byte data, and finally the data itself.//GR_replace
    glTexImage2D(GL_TEXTURE_2D, 0, 3, brick->w, brick->h, 0, GL_BGR, GL_UNSIGNED_BYTE, brick->pixels);//GR_replace

}

void Arkanoid::BuildFont()
{
 float cx,cy;//GR_replace
 int loop1;//GR_replace
	base=glGenLists(256);									// Creating 256 Display Lists//GR_replace
	glBindTexture(GL_TEXTURE_2D, tfont);				// Select Our Font Texture//GR_replace
	for (loop1=0; loop1<256; loop1++)						// Loop Through All 256 Lists//GR_replace
	{//GR_replace
		cx=(float)(loop1%16)/16.0f;						// X Position Of Current Character//GR_replace
		cy=(float)(loop1/16)/16.0f;						// Y Position Of Current Character//GR_replace
//GR_replace
		glNewList(base+loop1,GL_COMPILE);					// Start Building A List//GR_replace
			glBegin(GL_QUADS);								// Use A Quad For Each Character//GR_replace
				glTexCoord2f(cx,1.0f-cy-0.0625f);			// Texture Coord (Bottom Left)//GR_replace
				glVertex2d(0,16);							// Vertex Coord (Bottom Left)//GR_replace
				glTexCoord2f(cx+0.0625f,1.0f-cy-0.0625f);	// Texture Coord (Bottom Right)//GR_replace
				glVertex2i(16,16);							// Vertex Coord (Bottom Right)//GR_replace
				glTexCoord2f(cx+0.0625f,1.0f-cy);			// Texture Coord (Top Right)//GR_replace
				glVertex2i(16,0);							// Vertex Coord (Top Right)//GR_replace
				glTexCoord2f(cx,1.0f-cy);					// Texture Coord (Top Left)//GR_replace
				glVertex2i(0,0);							// Vertex Coord (Top Left)//GR_replace
			glEnd();										// Done Building Our Quad (Character)//GR_replace
			glTranslated(15,0,0);							// Move To The Right Of The Character//GR_replace
		glEndList();										// Done Building The Display List//GR_replace
	}														// Loop Until All 256 Are Built//GR_replace
}


GLvoid Arkanoid::KillFont(GLvoid)
{
 glDeleteLists(base,256);//GR_replace
}

GLvoid Arkanoid::glPrint(GLint x,GLint y,int set, const char *text)
{
	if (set>1)												// Did User Choose An Invalid Character Set?//GR_replace
	{//GR_replace
		set=1;												// If So, Select Set 1 (Italic)//GR_replace
	}//GR_replace
	glEnable(GL_TEXTURE_2D);								// Enable Texture Mapping//GR_replace
	glLoadIdentity();										// Reset The Modelview Matrix//GR_replace
	glTranslated(x,y,0);									// Position The Text (0,0 - Bottom Left)//GR_replace
	glListBase(base-32+(128*set));							// Choose The Font Set (0 or 1)//GR_replace
//GR_replace
	if (set==0)												// If Set 0 Is Being Used Enlarge Font//GR_replace
	{//GR_replace
		glScalef(1.5f,2.0f,1.0f);							// Enlarge Font Width And Height//GR_replace
	}//GR_replace
//GR_replace
	glCallLists(strlen(text),GL_UNSIGNED_BYTE, text);		// Write The Text To The Screen//GR_replace
	glDisable(GL_TEXTURE_2D);								// Disable Texture Mapping//GR_replace
};//GR_replace

void Arkanoid::NextLevel(){
	alevel->StuckBall();
	level++;
	if(level>NUM_LEVELS)
		game=false;
	else{
		static char pstr[]="lev1.txt";
		pstr[3]++;
		alevel->NewLevel(pstr);
	};
};
//--------------------------------------------------------------------------------------
void Stick::Update(float dtime){
	pos+=vec2(gArkanoid->GetStickVel(),0.0)*dtime;
	if(pos.x()<0)pos.x(0);
	if(pos.x()>NUM_COLS*COL_WIDTH)pos.x(NUM_COLS*COL_WIDTH);
};
//--------------------------------------------------------------------------------------
int main()
{
	gArkanoid=new Arkanoid();
	gArkanoid->Game();
	delete gArkanoid;
	return 0;
};
