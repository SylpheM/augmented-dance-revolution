#include "Application.hpp"

Application::Application()
: m_screen(NULL)
, m_camImage(NULL)
, m_windowsWidth(800)
, m_windowsHeight(600)
, m_camImageWidth(-1)
, m_camImageHeight(-1)
, m_isRunning(true)
, m_thres(50)
, m_config(NULL)
, viewCountB(0)
, viewCountC(0)
, viewCountBL(0)
, viewCountSL(0)
, viewCountFL(0)
, viewCountSR(0)
, viewCountFR(0)
, viewCountBR(0)
, bar(0)
, beat(-1)
, start(0)
, end(0)
, elapsed(-1.f)
, BPM_96(625.0)
, BPM_156(384.0)
, m_font(NULL)
, deltaTime(-1)
{

}

Application::~Application()
{
	arVideoCapStop();
    arVideoClose();

	Mix_CloseAudio();
	TTF_Quit();

	for(int i = 0; i < m_images.size(); ++i)
	{
		SDL_FreeSurface(m_images[i]->m_image);
		delete m_images[i]->m_pos;
		delete m_images[i]->m_size;
		delete m_images[i]->m_drawingSize;
		delete m_images[i];
	}

	SDL_FreeSurface(m_camImage);
	SDL_FreeSurface(m_screen);
	SDL_Quit();

	delete m_config;
}

//Initialise la fen�tre SDL et les param�tres ARToolkit
void Application::init()
{
	//SDL
	if(SDL_Init(SDL_INIT_VIDEO) != 0) 
	{
		std::cerr << "Can't initialize SDL : " << SDL_GetError() << std::endl;
		m_isRunning = false;
	}

	SDL_WM_SetCaption("AugmentedDanceRevolution", NULL);

	m_screen = SDL_SetVideoMode(m_windowsWidth, m_windowsHeight, 32, SDL_DOUBLEBUF|SDL_RESIZABLE);

	//SDL_ttf
	TTF_Init();
	m_font = TTF_OpenFont("../fonts/mvboli.ttf", 100);
	
	//Initialisation cam�ra
	#ifdef _WIN32
		char *vconf = "Data\\WDM_camera_flipV.xml";
	#else
		char *vconf = "";
	#endif

	ARParam  wparam;
    // open the video path
    if( arVideoOpen( vconf ) < 0 ) exit(0);
    // find the size of the window
    if( arVideoInqSize(&m_camImageWidth, &m_camImageHeight) < 0 ) exit(0);
    printf("Image size (x,y) = (%d,%d)\n", m_camImageWidth, m_camImageHeight);

    // set the initial camera parameters
	char *cparam_name = "Data/camera_para.dat";
	if( arParamLoad(cparam_name, 1, &wparam) < 0 ) {
        printf("Camera parameter load error !!\n");
        exit(0);
    }
	ARParam cparam;
    arParamChangeSize( &wparam, m_camImageWidth, m_camImageHeight, &cparam );
    arInitCparam( &cparam );
    printf("*** Camera Parameter ***\n");
    arParamDisp( &cparam );

	// New way multiple patterns
	char *config_name = "Data/multi/marker.dat";
	if( (m_config = arMultiReadConfigFile(config_name)) == NULL ) {
        printf("config data load error !!\n");
        exit(0);
    }

	initChoregraphy();

	//sound init
	bar = 0;
	beat = 0;
	deltaTime = BPM_96;
	
	//SDL_mixer
	if( Mix_OpenAudio(44100, MIX_DEFAULT_FORMAT, MIX_DEFAULT_CHANNELS, 1024) == -1)
		std::cout<<"problem init son"<<std::endl; //Initialisation de l'API Mixer
	
	Mix_Music * musique = Mix_LoadMUS("../musics/queen.ogg");
	Mix_VolumeMusic(MIX_MAX_VOLUME/2);
	if(musique == NULL){
		std::cout<<"musique non jou�e"<<std::endl;
	}
	else
	{
		//Mix_PlayMusic(musique, -1);
	}

	start = clock();
}

//Charge toutes les images de l'application
void Application::initImages()
{
	loadImage("../images/test.jpg", 0, 0, 512, 512, 30, 30);
}

//Charge une image et la stocke dans m_images
void Application::loadImage(const char * filename, int posX, int posY, int sizeX, int sizeY, int drawingSizeX, int drawingSizeY)
{
	Image * img = new Image;
	SDL_Surface * surface = IMG_Load(filename);
	img->m_image = surface;
	SDL_Rect * pos = new SDL_Rect;
	pos->x = posX;
	pos->y = posY;
	img->m_pos = pos;
	SDL_Rect * size = new SDL_Rect;
	size->x = sizeX;
	size->y = sizeY;	
	img->m_size = size;
	SDL_Rect * dSize = new SDL_Rect;
	dSize->x = drawingSizeX;
	dSize->y = drawingSizeY;	
	img->m_drawingSize = dSize;
	m_images.push_back(img);
}

//D�finit la boucle principale de l'application
void Application::run()
{
	init();
	initImages();
	arVideoCapStart();

	while(m_isRunning)
	{
		update();
	}
}

//G�re les op�rations de l'application
void Application::update()
{
	// ====== G�re les �v�nements SDL
	checkEvents();

	// ====== R�cup�re l'image film�e par la cam�ra
	ARUint8         *dataPtr;
    ARMarkerInfo    *marker_info;
    int             marker_num;
    int             i, j, k;

    // grab a vide frame
    if( (dataPtr = (ARUint8 *)arVideoGetImage()) == NULL ) {
        arUtilSleep(2);
        return;
    }

	//mirror the image camera
	int sizeX, sizeY;
	arVideoInqSize(&sizeX, &sizeY);
	ARUint8 * dataPtrFlipped = new ARUint8[sizeX*sizeY*AR_PIX_SIZE_DEFAULT];
	
	for(int i = 0; i < sizeY; ++i)
	{
		for(int j = 0, k = (sizeX * AR_PIX_SIZE_DEFAULT) - AR_PIX_SIZE_DEFAULT; j < sizeX * AR_PIX_SIZE_DEFAULT; j += AR_PIX_SIZE_DEFAULT, k-=AR_PIX_SIZE_DEFAULT)
		{
			dataPtrFlipped[i*sizeX * AR_PIX_SIZE_DEFAULT + j + 0] = dataPtr[i*sizeX * AR_PIX_SIZE_DEFAULT + k + 0];
			dataPtrFlipped[i*sizeX * AR_PIX_SIZE_DEFAULT + j + 1] = dataPtr[i*sizeX * AR_PIX_SIZE_DEFAULT + k + 1];
			dataPtrFlipped[i*sizeX * AR_PIX_SIZE_DEFAULT + j + 2] = dataPtr[i*sizeX * AR_PIX_SIZE_DEFAULT + k + 2];
			dataPtrFlipped[i*sizeX * AR_PIX_SIZE_DEFAULT + j + 3] = dataPtr[i*sizeX * AR_PIX_SIZE_DEFAULT + k + 3];
		}
	}

	//test save image
	Uint32 rmask, gmask, bmask, amask;
	rmask = 0x00000000;
	gmask = 0x00000000;
	bmask = 0x00000000;
	amask = 0x000000ff;
	m_camImage = SDL_CreateRGBSurfaceFrom(dataPtrFlipped, sizeX, sizeY, 8 * AR_PIX_SIZE_DEFAULT, sizeX * AR_PIX_SIZE_DEFAULT, rmask, gmask, bmask, amask);

    // detect the markers in the video frame 
	if( arDetectMarker(dataPtrFlipped, m_thres, &marker_info, &marker_num) < 0 ) {
		exit(0);
    }

	// ====== Gestion de la musique et du score
	end = clock();
	elapsed = ((double)end - start);

	if(elapsed >= deltaTime){
		 
		 beat++;
		 //Changement de mesure
		 if(beat == 5){
			 if (bar < 77) bar++;
			 viewCountB = 0;
			 viewCountBL = 0; 
			 viewCountBR = 0;
			 viewCountC = 0;
			 viewCountFL = 0;
			 viewCountFR = 0;
			 viewCountSL = 0;
			 viewCountSR = 0;
			 beat=1;
			 std::cout << "mesure " << bar << std::endl;
		 }
	
	if(bar == 13) deltaTime = BPM_156;

		// std::cout << "mesure " << bar;
		// std::cout << "beat " << beat << std::endl;
		 start = end;
	 }

	//MARQUE
	/* check for object visibility */
	//Detecte les marqueurs pr�sents dans move[] en fonction de la mesure du morceau
 	for( i = 0; i < move[bar].size(); i++ ) {
		k = -1;
		for( j = 0; j < marker_num; j++ ) {

			if( move[bar].at(i) == marker_info[j].id ) {
				/* you've found a pattern */
				printf("Found pattern: %d ", marker_info[j].id);
				switch(marker_info[j].id){

				case C: {
					printf("Chest"); 
					viewCountC++;
					break;	
					};
				
				case B: {
					printf("Back");
					viewCountB++;
					break;
				}
				case SR: {
					
					printf("Shoulder Right");
					viewCountSR++;
					break;
				}
				case SL: {
					printf("Shoulder Left"); 
					viewCountSL++;
					break;
						 }
				case FR: {
					printf("Hand Right Front"); 
					viewCountFR++;
					break;
						 }
				case BR: {
						printf("Hand Right Back"); 
						viewCountBR++;
						break;
					}
				case FL: {
					printf("Hand Left Front"); 
					viewCountFL++;
					break;
						 }
				case BL: {
					printf("Hand Left Back"); 
					viewCountBL++;
					break;
					 }

				};
				printf("\n");
				//glColor3f( 0.0, 1.0, 0.0 );
				argDrawSquare(marker_info[j].vertex,0,0);
				if( k == -1 ) k = j;
				else if( marker_info[k].cf < marker_info[j].cf ) k = j;
			}
		}

		if( k == -1 ) {
			m_config->marker[i].visible = 0;
			continue;
		}

		
		checkPosition();

		/* calculate the transform for each marker */
		if( m_config->marker[i].visible == 0 ) {
            arGetTransMat(&marker_info[k], m_config->marker[i].center, m_config->marker[i].width, m_config->marker[i].trans);
        }
        else {
            arGetTransMatCont(&marker_info[k], m_config->marker[i].trans, m_config->marker[i].center, m_config->marker[i].width, m_config->marker[i].trans);
        }
        m_config->marker[i].visible = 1;
		
	}


	render();
	arVideoCapNext();

	delete []dataPtrFlipped;
}

//G�re le rendu de l'application
void Application::render()
{
	// ===== Affichage de l'image film�e par la cam�ra
	float zoomX = (float)m_windowsWidth / m_camImageWidth;
	float zoomY = (float)m_windowsHeight / m_camImageHeight;
	
	if(m_camImage != NULL)
	{
		SDL_Surface * camImage = rotozoomSurfaceXY(m_camImage, 0.0f, zoomX, zoomY, 1);

		SDL_Rect pos;
		pos.x = 0;
		pos.y = 0;
		SDL_BlitSurface(camImage, NULL, m_screen, &pos);
		SDL_FreeSurface(camImage);
	}

	// ===== Affichage des �l�ments graphiques lors de la d�tection d'un marqueur
	int     i;
    double  gl_para[16];

    /* calculate the viewing parameters - gl_para */
	for( i = 0; i < m_config->marker_num; i++ )
	{
        if( m_config->marker[i].visible == 1 )
		{
			//argConvGlpara(m_config->marker[i].trans, gl_para);
			drawObject(i);
		}
    }

	// ===== Affichage des textes
	SDL_Color green = {0, 255, 0};
	drawText(500, 200, green, "HEllo!");

	// ===== Affichage des �lements de la chor�graphie
	drawImage(0, 600, 500);

	// ===== Met � jour l'affichage
	SDL_Flip(m_screen);
	SDL_UpdateRect(m_screen, 0, 0, m_windowsWidth, m_windowsHeight);
}

//Dessine une image � la position du marqueur idMarker
void Application::drawObject(int idMarker)
{
	//R�cup�ration des coordonn�es du marqueur sur l'image
	SDL_Rect pos;
	pos.x = m_config->marker[idMarker].trans[2][0];
	pos.y = m_config->marker[idMarker].trans[2][1];
	int z = abs(m_config->marker[idMarker].trans[2][2] - 200);
	int maxZ = 1500;
	double rapport = (maxZ-z)/maxZ;

	float zoomX = (float) (m_images[0]->m_size->x * rapport) / m_images[0]->m_size->x;
	float zoomY = (float) (m_images[0]->m_size->y * rapport) / m_images[0]->m_size->y;
	
	SDL_Surface * img = rotozoomSurfaceXY(m_images[0]->m_image, 0.0f, zoomX, zoomY, 1);
	SDL_BlitSurface(img, NULL, m_screen, &pos);
	SDL_FreeSurface(img);
}

//Dessine le texte s � la position (x, y) de la fen�tre
void Application::drawText(int x, int y, SDL_Color color, char* s)
{
	SDL_Surface * texte = TTF_RenderText_Blended(m_font, s, color);
	SDL_Rect pos;
	pos.x = x;
	pos.y = y;
	SDL_BlitSurface(texte, NULL, m_screen, &pos);
	SDL_FreeSurface(texte);
}

//Dessine l'image id � la position (x, y) de la fen�tre
void Application::drawImage(int id, int x, int y)
{
	if( id < m_images.size() )
	{
		if(m_images[id]->m_image)
		{
			float zoomX = (float) m_images[id]->m_drawingSize->x / m_images[id]->m_size->x;
			float zoomY = (float) m_images[id]->m_drawingSize->y / m_images[id]->m_size->y;
	
			SDL_Surface * img = rotozoomSurfaceXY(m_images[id]->m_image, 0.0f, zoomX, zoomY, 1);

			SDL_Rect pos;
			pos.x = x;
			pos.y = y;
			SDL_BlitSurface(img, NULL, m_screen, &pos);
			SDL_FreeSurface(img);
		}
	}
}

//G�re les �v�nements SDL de l'application
void Application::checkEvents()
{
	SDL_Event event;
	if( SDL_PollEvent(&event) )
	{
		switch (event.type)
		{
			case SDL_QUIT:
				m_isRunning = false;
				break;
			case SDL_VIDEORESIZE:
				m_windowsWidth = event.resize.w;
				m_windowsHeight = event.resize.h;
				m_screen = SDL_SetVideoMode(m_windowsWidth, m_windowsHeight, 32, SDL_VIDEORESIZE | SDL_DOUBLEBUF);
				break;
			case SDL_KEYDOWN:
				switch(event.key.keysym.sym)
				{
					case SDLK_ESCAPE:
						m_isRunning = false;
						break;
				}
				break;
		}
	}
}

void Application::initChoregraphy()
{
	//77 = nombre de mesures du morceau (+ ou -)
	//* 4 = signature rythmique du morceau. 
	//Avec le tableau move on peut potentiellement faire un chagement de mouvement � chaque temps du morceau

	for(int i = 0; i < 77*4; i++)
	{
		move[i].push_back(Marker::B);
		move[i].push_back(Marker::C);
		move[i].push_back(Marker::BL);
		move[i].push_back(Marker::BR);
		move[i].push_back(Marker::FL);
		move[i].push_back(Marker::SR);
		move[i].push_back(Marker::SL);
		move[i].push_back(Marker::FR);
	}
}

void Application::checkPosition()
{
	unsigned int threshold = 0; 
	std::vector<bool> posOK;

	int i =0;
	while(i < move[bar].size()){

		switch(move[bar].at(i)){
			case C : 
				if(viewCountC > threshold) posOK.push_back(true);
				else posOK.push_back(false);
				break;
			case B : 
				if(viewCountB > threshold) posOK.push_back(true);
				else posOK.push_back(false);
				break;
			case BL : 
				if(viewCountBL > threshold) posOK.push_back(true);
				else posOK.push_back(false);
				break;
		}//end switch

	i++;
	}//end while

	bool checker= true;
	for(int j =0; j < posOK.size(); j++){

		checker = checker && posOK.at(j);
	}

	std::cout << "Check Pos" << checker << std::endl;
}