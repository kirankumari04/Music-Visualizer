#include <GL/glut.h>
#include <GL/freeglut.h>
#include <iostream>
#include <cstdlib>
#include <stdio.h>
#include <memory.h>
#include <cmath>
#include <fftw3.h>
#include "sndfile.h"
#include "portaudio.h"

using namespace std;

//Function Declarations
void display();
void transformed_display();
void timeDomain();
void reshape();
void init();
void init_audio();
void close_audio();
float lowpassFilter(float *sample, int size);
void windowFunction(float *sample, int size);

#define SAMPLE_RATE 441000
#define WIN 1024
#define buffer_size 1024
#define PI 3.14159265358979323846264338327950288
const int fft_size = 512;
double windowedSample[WIN];
float *buf, leftch[WIN], rightch[WIN];
int readSize=0, len;//to strore the size of current frame for the display func to pass to fft
static char CHANNEL[10];//MONO or STEREO
bool finished=false;//finished playing or not
int eff=1;//variable indicating the theme number
int channel=1;
double bartop[fft_size], radii[5];
char audpath[100] = "music_files/Horizon.wav" ;//demo file
PaStream *stream;//portaudio stream


GLsizei width = 800;
GLsizei height = 600;
GLsizei last_width = width;
GLsizei last_height = height;
// light 0 position
GLfloat light0_pos[4] = {2.0f, 1.2f, 4.0f, 1.0f};
// light 1 parameters
GLfloat light1_ambient[] = {0.2f, 0.2f, 0.2f, 1.0f};
GLfloat light1_diffuse[] = {1.0f, 1.0f, 1.0f, 1.0f};
GLfloat light1_specular[] = {1.0f, 1.0f, 1.0f, 1.0f};
GLfloat light1_pos[4] = {-2.0f, 0.0f, -4.0f, 1.0f};


struct layer
{
	float radius;
	float zoom;
	struct layer *next;
	struct layer *prev;
};


typedef layer layers;
layers *head=NULL;

void idle()
{
	glutPostRedisplay();
}

void init()
{
    glClearColor(0.0f, 0.05f,0.05f, 0.3f);
    glShadeModel(GL_SMOOTH);
    glEnable(GL_DEPTH_TEST);
    
    glFrontFace(GL_CCW);
    
    glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
    
    glEnable(GL_LIGHTING);
    
    glLightModeli(GL_FRONT_AND_BACK, GL_TRUE);
     
    glColorMaterial(GL_FRONT_AND_BACK, GL_AMBIENT_AND_DIFFUSE);
    
    glEnable(GL_COLOR_MATERIAL);

    
    glEnable(GL_LIGHT0);

    glLightfv(GL_LIGHT1, GL_AMBIENT, light1_ambient);
    glLightfv(GL_LIGHT1, GL_DIFFUSE, light1_diffuse);
    glLightfv(GL_LIGHT1, GL_SPECULAR, light1_specular);
    glEnable(GL_LIGHT1);
	
}

void reshape(int w, int h)
{
	width = w; height = h;
    
    glViewport(0, 0, w, h);
    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    
    gluPerspective(45.0, (GLfloat)(w)/(GLfloat)(h), 1.0, 300.0);
    
    glMatrixMode(GL_MODELVIEW);
    glLoadIdentity();
    
    gluLookAt(0.0f, 0.0f, 3.5f, 0.0f, 0.0f, 0.0f, 0.0f, 1.0f, 0.0f);
		
    
    glLightfv(GL_LIGHT0, GL_POSITION, light0_pos);
    glLightfv(GL_LIGHT1, GL_POSITION, light1_pos);
}

void display()
{
	glClear( GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT );
	glClearColor( 0.0f, 0.08f,0.08f, 0.3f );

	//printing now playing stuff...
	static char now[1024], str1[1024];
	int foo, len=strlen(audpath), foobar=0;
	for(foo=0;foo<len;foo++)
		if(audpath[foo]=='/')
			foobar=foo+1;
	int ind=0;
	for(int bar=foobar;bar<len-4;bar++)
		str1[ind++]=audpath[bar];

	str1[ind]='\0';
	
	sprintf( now, "Now Playing : %s || Channel: %s", str1, CHANNEL);
	char *ch = now;
	glPushMatrix();
	glTranslatef(-1.8, 1.2, 0.0);
	glScalef( 0.001f, .0006f, .01f);
	glPointSize(0.5);
	for (ch=now; *ch != '\0'; ch++)
		glutStrokeCharacter(GLUT_STROKE_ROMAN, *ch);

	glPopMatrix();

	if(eff==1)//first theme: time domain waveform
		timeDomain();
	else
		transformed_display();

	if(finished==true)
	{
		close_audio();
		exit(0);
	}
}

void timeDomain()
{
	GLfloat x = -2.0f, inc = 4.0f / readSize, y = 0.0f;

	glPushMatrix();
	glColor3f( 0.4f, 0.4f, 1.0f );		
	glNormal3f( 0.0f, 0.0f, 1.0f );// set vertex normals (for somewhat controlled lighting)
	glTranslatef(x, y, 0.0f);
	glScalef( inc, 1.0, 1.0 );

	glBegin(GL_LINE_STRIP);//drawing the timedomain waveform
		for(int i=0; i<readSize; i++)
			glVertex2f( x++ , buf[i] );
	glEnd();
	glPopMatrix();

	glColor3f(1,1,1);
	glFlush();
	glutSwapBuffers();
}

float lowpassFilter(int size)
{
	static float lastInput = 0;
	float output = ( *buf + lastInput ) / 2 ;
	lastInput = *buf;
	return output;
}

void windowFunction(int size)//Hanning window function
{
	for (int i = 0; i < size; i++) 
	{
		double multiplier = 0.5 * (1 - cos(2*PI*i/(size-1)));
		windowedSample[i]=buf[i]*multiplier;
	}
	
}


struct OurData//sound data
{
  SNDFILE *sndFile;//the file
  SF_INFO sfInfo;//information about the file
  int position;//index to the file
};


//performing fft and drawing frequency domain waveform
void transformed_display()
{
	if(channel==2)
	{
		int k=0;
		for(int i=0;i<readSize;i++)
		{
			leftch[k]=buf[i];
			rightch[k]=buf[i+1];
			k++;
		}
		len = k;
	}
	else
	{
		for(int i=0;i<readSize;i++)
		{
			leftch[i]=buf[i];
			rightch[i]=leftch[i];
		}
		len = readSize;
	}

	int fftSize=fft_size;
	if(channel==1) fftSize/=2;

	lowpassFilter(fftSize);
	windowFunction(fftSize);

	fftw_complex *out;
	out = (fftw_complex*) fftw_malloc(sizeof(fftw_complex) * fftSize);
    fftw_plan p;
    p = fftw_plan_dft_r2c_1d(fftSize, windowedSample, out, FFTW_ESTIMATE);

    fftw_execute(p);
    fftw_destroy_plan(p); 

	float powerSpectrum[fft_size/2], dbSpectrum[fft_size/2], normSpect[fft_size/2];

	for (int i=0; i<fftSize/2; i++) {
        powerSpectrum[i] = 0.0;
		dbSpectrum[i]=0.0;
    }

	int minm=0, maxm=0;
	float mam=0;
    for (int i=0; i<fftSize/(channel*2); i++) {
        powerSpectrum[i] = sqrt(out[i][0]*out[i][0] + out[i][1]*out[i][1]);
		if(powerSpectrum[i]>powerSpectrum[maxm]) maxm=i;
		dbSpectrum[i]=20.0 * log10(powerSpectrum[i]);
		normSpect[i]=dbSpectrum[i]/(fftSize/2);
		if(normSpect[i]<normSpect[minm]) minm=i;
		if(normSpect[i]>mam) mam=normSpect[i];
    }
	
	float binSize=SAMPLE_RATE/fftSize;
	float peakFreq=maxm*binSize;
	//if(channel==1)
		//peakFreq/=2.0;

	if(eff==2)//drawing frequency spectrum bars
	{
		GLfloat x = -2.0f, inc, y = 0.0f;
		x = -2.0f;
		inc = 4.0f / (fftSize/(channel*2));
        y=0.6*inc;

		glPushMatrix();
		glTranslatef(0,-1,0);
		glScalef(1, 3, 1);
		
		glPointSize(3);
		
		GLfloat base=abs(normSpect[minm]);
		for(int j=0;j<fftSize/(channel*2);j++)
		{	
			int foo=int(normSpect[j]);//used for controlling 
			float foo1=float(normSpect[j]-(float)(foo));//the colors of the bars
				
			glBegin(GL_QUADS);
				glColor3f(1,0,0.5);
				glVertex3f(x,0,0);

				glColor3f(0.5+foo1,0,0.7+foo1);
				glVertex3f(x,base+normSpect[j],0);

				glColor3f(0.5+foo1,0,0.7+foo1);
				glVertex3f(x+y,base+normSpect[j],0);

				glColor3f(1,0,0.5);
				glVertex3f(x+y,0,0);
			glEnd();
			if(bartop[j]<base+normSpect[j]) 
				bartop[j]=base+normSpect[j];
			else
				bartop[j]=bartop[j]-0.0008;

			glColor3f(1,0,0.3);
			glBegin(GL_QUADS);	
				glVertex3f(x, bartop[j], 0);
				glVertex3f(x, bartop[j]+0.005, 0);
				glVertex3f(x+y, bartop[j]+0.005, 0);
				glVertex3f(x+y, bartop[j], 0);
			glEnd();
				
			x+=inc;
		
		}
		glPopMatrix();
	
		glColor3f( 1, 1, 1 );
	}
	else if(eff==3)//Third theme (peak frequencies and left-right channel waveforms)
	{
		//adding the newest peak frequency (i.e. the current one)
		layers *cur = new layers;
		cur->radius=peakFreq/10000;
		cur->zoom=1.0;
		cur->next=head;
		if(head!=NULL)
			head->prev=cur;
		cur->prev=NULL;
		head=cur;

		glColor3f(0, 0.5, 0.7);

		//The circle for "holding" the waveform
		layers *t = head;
		float v;
		v = t->radius;
		while(v>1.0)
			v-=1.0;
		v+=1;
		glPushMatrix();
		glBegin(GL_LINE_STRIP);
			for(float i=0;i<=360;i++)
			{
				glVertex2f(v*cos(i*PI/180), v*sin(i*PI/180));
			}
		glEnd();
		glPopMatrix();


		//The peak freq layers
		float val;
		layers *temp = head;
		while(temp!=NULL)
		{
				glPushMatrix();
				if(channel==1) glScalef(0.25f,0.25f,0.25f);
				glBegin(GL_LINE_STRIP);
					for(float i=0;i<=360;i++)
					{
						val = temp->radius*temp->zoom;
						
						glVertex2f(val*cos(i*PI/180), val*sin(i*PI/180));
					}
				glEnd();
				glPopMatrix();
			temp->zoom+=0.010;
			if(temp->zoom>=2.0)
			{
				temp->next=NULL;
				temp->prev->next=temp->next;
			}
			temp=temp->next;
		}

		//The waveform on the circle
		float ang = float(360.0/(float)len), angle = 0, u;
		glColor3f(0,float(rand())/float(RAND_MAX),0.7);
		glPushMatrix();
		glBegin(GL_LINES);
			for(int i=0;i<len;i++)
			{
				u = abs(leftch[i]);
				if(mam>0.1) u/=2.0;
				u = abs(v-u);
				glVertex2f(v*cos(angle*PI/180), v*sin(angle*PI/180));
				glVertex2f((u)*cos(angle*PI/180), (u)*sin(angle*PI/180));
				angle+=ang;
			}
		glEnd();
		glPopMatrix();
		

		//The horizontal waveform in the background
		GLfloat x = -2.0f, inc = 4.0f / len, y = 0.0f;
		glPushMatrix();
		glColor3f(0,1,0);
		if(mam>0.1)
			glScalef( 1.0, 0.5f, 1.0 );

		glBegin(GL_LINES);
			for(int i=0; i<len; i++)
			{
				glVertex2f( x , 0);
				glVertex2f( x , abs(leftch[i]) );
				glVertex2f( x , 0);
				glVertex2f( x , 0-abs(rightch[i]) );
				
				x+=inc;
			}
			
		glEnd();
		glPopMatrix();

		glColor3f(1,1,1);
	}

	glFlush();
	glutSwapBuffers();
}


int Callback(const void *input,
             void *output,
             unsigned long frameCount,
             const PaStreamCallbackTimeInfo* paTimeInfo,
             PaStreamCallbackFlags statusFlags,
             void *userData)
{
  OurData *data = (OurData *)userData; /* we passed a data structure
into the callback so we have something to work with */
  float *cursor; /* current pointer into the output  */
  float *out = (float *)output;
  int thisSize = frameCount;
  int nFrames=frameCount;
  int thisRead;

  cursor = out; /* set the output cursor to the beginning */
  while (thisSize > 0)
  {
    /* seek to our current file position */
    sf_seek(data->sndFile, data->position, SEEK_SET);

    /* are we going to read past the end of the file?*/
    if (thisSize > (data->sfInfo.frames - data->position))
    {
      /*if we are, only read to the end of the file*/
      thisRead = data->sfInfo.frames - data->position;
      /* and then loop to the beginning of the file */
      data->position = 0;
    }
    else
    {
      /* otherwise, we'll just fill up the rest of the output buffer */
      thisRead = thisSize;
      /* and increment the file position */
      data->position += thisRead;
    }

    /* since our output format and channel interleaving is the same as
	sf_readf_int's requirements */
    /* we'll just read straight into the output buffer */
    nFrames=sf_readf_float(data->sndFile, cursor, thisRead);
	readSize=thisRead;
	buf=cursor;
	if(nFrames<frameCount)
	{
		finished=true;
		return paComplete;
	}

	
    /* increment the output cursor*/
    cursor += thisRead;
    /* decrement the number of samples left to process */
    thisSize -= thisRead;	
	
  }

  return paContinue;
}


void init_audio()
{
  OurData *data = (OurData *)malloc(sizeof(OurData));
  
  PaError error;
  PaStreamParameters outputParameters;

  data->position = 0;
  data->sfInfo.format = 0;
  
  data->sndFile = sf_open(audpath, SFM_READ, &data->sfInfo);

  if (!data->sndFile)
  {
    cout<<"Error opening file. Please check if the file exists at the specified location.\n";
	getchar();
	getchar();
    exit(1);
  }


  if(data->sfInfo.channels==2)
	  channel=2;
	  
  for(int i=0;i<fft_size;i++)
	  bartop[i]=-1.0;

  /* start portaudio */
  Pa_Initialize();

  /* set the output parameters */
  outputParameters.device = Pa_GetDefaultOutputDevice();
  outputParameters.channelCount = data->sfInfo.channels;
  if(data->sfInfo.channels==1) strcpy(CHANNEL,"MONO"); else strcpy(CHANNEL,"STEREO");
  outputParameters.sampleFormat = paFloat32;
  outputParameters.suggestedLatency = 0.2;
  outputParameters.hostApiSpecificStreamInfo = 0;

  
  error = Pa_OpenStream(&stream, 0, &outputParameters, data->sfInfo.samplerate,	WIN, paNoFlag, Callback, data );

  if (error)
  {
    printf("error opening output, error code = %i\n", error);
    Pa_Terminate();
    exit(1);
  }

  /* when we start the stream, the callback starts getting called */
  Pa_StartStream(stream);
  
}


void close_audio()
{
	Pa_StopStream(stream);
	Pa_CloseStream(stream);
	Pa_Terminate();
}


void keyboard(unsigned char key, int x, int y)
{
	if(key=='t'||key=='T') eff=1;
	if(key=='f'||key=='F') eff=2;
	if(key=='m'||key=='M') eff=3;
}


int main(int argc, char **argv)
{	
	char opt;
	cout<<"Play demo file?(y/n): ";
	cin>>opt;
	if(opt=='n')
	{
		cout<<"Enter the path of a .wav file: ";
		cin>>audpath;
	}
	init_audio();
	cout<<"Themes: \nt - timedomain waveform\nf - frequency bars\nm - misc";
	glutInit(&argc, argv);
	glutInitDisplayMode(GLUT_SINGLE|GLUT_RGB|GLUT_DEPTH);
	glutInitWindowSize(width, height);
	glutInitWindowPosition(100, 100);
	glutCreateWindow("Music Visualizer");
	glutDisplayFunc(display);
	glutReshapeFunc(reshape);
	glutKeyboardFunc(keyboard);
	glutIdleFunc(idle);
	init();
	glutMainLoop();
	return 0;
}