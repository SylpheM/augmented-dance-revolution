#ifndef __APPLICATION_HPP__
#define __APPLICATION_HPP__

#ifdef _WIN32
#include <windows.h>

#include "AR/gsub.h"
#include "AR/video.h"
#include "AR/param.h"
#include "AR/ar.h"
#include "AR/arMulti.h"

#include "time.h"

#include "SDL/SDL_mixer.h"
#include "SDL/SDL_image.h"
#include "SDL/SDL_rotozoom.h"
#include "SDL/SDL_ttf.h"
#else
#include <AR/gsub.h>
#include <AR/video.h>
#include <AR/param.h>
#include <AR/ar.h>
#include <AR/arMulti.h>

#include <time.h>

#include <SDL/SDL_mixer.h>
#include <SDL/SDL_image.h>
#include <SDL/SDL_rotozoom.h>
#include <SDL/SDL_ttf.h>

#endif

#include <iostream>
#include <vector>

enum Marker {C, B, SR, SL, FR, BR, FL, BL};

typedef struct image
{
	SDL_Surface * m_image;
	SDL_Rect * m_pos;
	SDL_Rect * m_size;
} Image;

class Application
{
	public :
		Application();
		~Application();

		//Initialise la fen�tre SDL et les param�tres ARToolkit
		void init();
		//Charge toutes les images de l'application
		void initImages();
		//Charge une image et la stocke dans m_images, drawingSizeX & drawingSizeY sont en % de la taille de la fen�tre
		void loadImage(const char * filename, int posX, int posY, int sizeX, int sizeY);

		//Affiche et met � jour les silhouettes et le score
		void updateInterface();

		//D�finit la boucle principale de l'application
		void run();
		//G�re les op�rations de l'application
		void update();
		//G�re le rendu de l'application
		void render();

		//Dessine des images sur les marqueurs de m_markersToDraw
		void drawMarkers();
		//Dessine une image � la position du marqueur idMarker
		void drawMarker(int idMarker);
		//Dessine le texte s � la position (x, y) de la fen�tre
		void drawText(int x, int y, SDL_Color color, char* s);
		//Attribue la position (*i, *j) � une image et la trace. Si i ou j est nul, l'image sera trac�e � sa position de base.
		void drawImage(int id, int * i, int * j);

		//G�re les �v�nements SDL de l'application
		void checkEvents();

		void initChoregraphy();
		void checkPosition();

	private :
		bool							m_gameStarted;
		SDL_Surface *					m_screen;
		SDL_Surface *					m_camImage;
		int								m_windowsWidth;
		int								m_windowsHeight;
		int								m_camImageWidth;
		int								m_camImageHeight;
		bool							m_isRunning;

		const int						m_thres;
		ARMultiMarkerInfoT *			m_config;
		ARMarkerInfo *					m_marker_info;
		std::vector<Image*>				m_images;
				
		//Stocke la liste des marqueur � identifier en fonction de la mesure courante du morceau
		std::vector<Marker>				move[77];
		std::vector<int>				imagesMove;
		std::vector<int>				m_markersToDraw;

		//viewCount stocke le nombre de fois qu'un marqueur a �t� vu dans une mesure 
		unsigned int					viewCountB, viewCountC, viewCountBL, viewCountSL, viewCountFL, viewCountSR, viewCountFR, viewCountBR;

		unsigned int					bar, beat, score;
		clock_t							start, end;
		double							elapsed;
		const double					BPM_96;
		const double					BPM_156;
		Mix_Chunk*						validate;
		Mix_Chunk*						musique;
		bool							moveDone;

		//Score
		TTF_Font *						m_font;

		//Stocke le temps a attendre entre 2 beat
		double							deltaTime;
};

#endif //__APPLICATION_HPP__