#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <GL/freeglut.h>
#include <string.h> // Para sprintf

#define TYPES 4
#define MAX_LIXOS 12
#define VIDAS_ORIGINAIS 8
#define BONUS_VIDAS 3
#define INTERVALO_BONUS 250

// =================================================================
// NOVO: Gerenciamento de estado do jogo e menu
// =================================================================
typedef enum { MENU, JOGO, CONTROLES, SOBRE } GameState;
GameState estadoJogo = MENU;
int opcaoMenu = 0; // 0: Iniciar, 1: Controles, 2: Sobre

float velocidadeLixo = 0.2; // velocidade inicial

typedef struct {
    GLfloat x, y;
    GLfloat r, g, b;
    int tipo; // 0=plástico, 1=vidro, 2=papel, 3=metal
} Lixo;

GLfloat wid = 25, hei = 25;
GLint estadosLata = 0;
GLfloat rLata = 1, gLata = 0, bLata = 0;
GLfloat xLata = 0;
GLint windowWidth = 500;
GLint intervalo = 1000;

Lixo lixos[MAX_LIXOS];
int numLixos = 0;
// tempo em que ocorreu o último spawn
int ultimoSpawn = 0;
int pontuacao = 0;
int vidas = VIDAS_ORIGINAIS; // Número de vidas do jogador
// garante que 3 vidas são adicionadas a cada 250 pontos
int ultimoBonus = 0;

int randInt(int max) {
    return rand() % max;
}

void geraCorLixo(GLfloat *r, GLfloat *g, GLfloat *b, int *tipo) {
    *tipo = randInt(TYPES);
    switch (*tipo) {
        case 0: *r = 1; *g = 0; *b = 0; break;   // Plástico (Vermelho)
        case 1: *r = 0; *g = 0.5; *b = 0; break; // Vidro (Verde)
        case 2: *r = 0; *g = 0; *b = 1; break;   // Papel (Azul)
        case 3: *r = 1; *g = 1; *b = 0; break;   // Metal (Amarelo)
    }
}

void novoLixo() {
    if (numLixos < MAX_LIXOS) {
        // define a posição de spawn no eixo x, além de acompanhar a largura da janela
        int randX = randInt((int)(2 * wid));
        // evita que o lixo apareça exatamente nos dois cantos da tela, impossibilitando
        // que ele seja pego
        lixos[numLixos].x = -wid + (randX ? (randX == wid ? randX - 3 : randX) : 3);
        lixos[numLixos].y = hei-1;
        geraCorLixo(&lixos[numLixos].r, &lixos[numLixos].g, &lixos[numLixos].b, &lixos[numLixos].tipo);
        
        numLixos++;
    }
}

void desenhaTexto(float x, float y, char *texto) {
    glRasterPos2f(x, y);
    for (char *c = texto; *c != '\0'; c++) {
        glutBitmapCharacter(GLUT_BITMAP_HELVETICA_18, *c);
    }
}

// =================================================================
// Funções de desenho para cada tipo de lixo
// =================================================================

// Desenha uma garrafa PET para representar o plástico
void desenhaPlastico() {
    glColor3f(1.0f, 0.2f, 0.2f);
    glBegin(GL_QUADS);
        glVertex2f(-1.0f, -2.0f); glVertex2f(1.0f, -2.0f);
        glVertex2f(1.0f, 0.5f); glVertex2f(-1.0f, 0.5f);
    glEnd();
    glBegin(GL_TRIANGLES);
        glVertex2f(-1.0f, 0.5f); glVertex2f(1.0f, 0.5f); glVertex2f(0.5f, 1.5f);
        glVertex2f(-1.0f, 0.5f); glVertex2f(-0.5f, 1.5f); glVertex2f(0.5f, 1.5f);
    glEnd();
    glColor3f(0.8f, 0.0f, 0.0f);
    glBegin(GL_QUADS);
        glVertex2f(-0.4f, 1.5f); glVertex2f(0.4f, 1.5f);
        glVertex2f(0.4f, 1.9f); glVertex2f(-0.4f, 1.9f);
    glEnd();
}

// Desenha uma garrafa de vidro quebrada
void desenhaVidro() {
    glColor3f(0.1f, 0.4f, 0.1f);
    glBegin(GL_TRIANGLES);
        glVertex2f(-1.5f, -1.5f); glVertex2f(0.0f, -1.0f); glVertex2f(-1.0f, 0.5f);
        glVertex2f(0.2f, -2.0f); glVertex2f(1.8f, -1.8f); glVertex2f(1.5f, 0.0f);
        glVertex2f(-0.5f, 0.8f); glVertex2f(1.0f, 1.2f); glVertex2f(0.0f, 2.0f);
    glEnd();
}

// Desenha uma bola de papel amassado
void desenhaPapel() {
    glColor3f(0.5f, 0.5f, 1.0f);
    glBegin(GL_POLYGON);
        glVertex2f(-1.5f, -1.0f); glVertex2f(0.0f, -1.8f); glVertex2f(1.5f, -0.5f);
        glVertex2f(1.2f, 1.0f); glVertex2f(0.0f, 1.5f); glVertex2f(-1.2f, 1.2f);
    glEnd();
    glColor3f(0.2f, 0.2f, 0.8f);
    glLineWidth(2);
    glBegin(GL_LINE_LOOP);
        glVertex2f(-1.5f, -1.0f); glVertex2f(0.0f, -1.8f); glVertex2f(1.5f, -0.5f);
        glVertex2f(1.2f, 1.0f); glVertex2f(0.0f, 1.5f); glVertex2f(-1.2f, 1.2f);
    glEnd();
}

// Desenha uma lata de metal
void desenhaMetal() {
    glColor3f(1.0f, 1.0f, 0.0f);
    glBegin(GL_QUADS);
        glVertex2f(-1.2f, -1.5f); glVertex2f(1.2f, -1.5f);
        glVertex2f(1.2f, 1.5f); glVertex2f(-1.2f, 1.5f);
    glEnd();
    glColor3f(0.7f, 0.7f, 0.7f);
    glBegin(GL_QUADS);
        glVertex2f(-1.25f, -0.5f); glVertex2f(1.25f, -0.5f);
        glVertex2f(1.25f, 0.5f); glVertex2f(-1.25f, 0.5f);
    glEnd();
    glColor3f(0.6f, 0.6f, 0.6f);
    glBegin(GL_QUADS);
        glVertex2f(-1.2f, 1.5f); glVertex2f(1.2f, 1.5f);
        glVertex2f(1.2f, 1.7f); glVertex2f(-1.2f, 1.7f);
        glVertex2f(-1.2f, -1.7f); glVertex2f(1.2f, -1.7f);
        glVertex2f(1.2f, -1.5f); glVertex2f(-1.2f, -1.5f);
    glEnd();
}

void atualizaLixos() {
    for (int i = 0; i < numLixos; i++) {
        lixos[i].y -= velocidadeLixo;
        
        if (lixos[i].y < -12 && lixos[i].y > -20) {
            if (lixos[i].x > xLata - 3 && lixos[i].x < xLata + 3) {
                if (lixos[i].tipo == estadosLata) pontuacao += 10;
                else vidas--;
                
                for (int j = i; j < numLixos - 1; j++) lixos[j] = lixos[j + 1];
                numLixos--; i--; continue;
            }
        }
        
        if (lixos[i].y < -20) {
            vidas--;
            for (int j = i; j < numLixos - 1; j++) lixos[j] = lixos[j + 1];
            numLixos--; i--;
        }
    }
}

GLfloat converteXMouse(int x) {
    GLfloat xConvertido = -wid + (2 * wid * (GLfloat)x / windowWidth);
    GLfloat limiteEsquerdo = -wid + 3, limiteDireito = wid - 3;
    if (xConvertido < limiteEsquerdo) xConvertido = limiteEsquerdo;
    else if (xConvertido > limiteDireito) xConvertido = limiteDireito;
    return xConvertido;
}

void desenhaLata() {
    glColor3f(rLata, gLata, bLata);
    glBegin(GL_QUADS);
        glVertex2f(-3, -19.8); glVertex2f(3, -19.8);
        glVertex2f(3, -12); glVertex2f(-3, -12);
    glEnd();
}

void desenhaChao() {
    glColor3f(0, 1, 0);
    glLineWidth(3);
    glBegin(GL_LINES);
        glVertex2f(-wid, -20); glVertex2f(wid, -20);
    glEnd();
}

void desenhaLixos() {
    for (int i = 0; i < numLixos; i++) {
        glPushMatrix();
        glTranslatef(lixos[i].x, lixos[i].y, 0);
        glScalef(1.5, 1.5, 1.5);
        switch (lixos[i].tipo) {
            case 0: desenhaPlastico(); break;
            case 1: desenhaVidro(); break;
            case 2: desenhaPapel(); break;
            case 3: desenhaMetal(); break;
        }
        glPopMatrix();
    }
}

// =================================================================
// NOVO: Funções de desenho para as telas do menu
// =================================================================

void desenhaMenu() {
    glColor3f(0, 0, 0);
    desenhaTexto(-8, 15, "RECYCLE RUSH");

    // Opções do menu com destaque para a selecionada
    if (opcaoMenu == 0) glColor3f(1, 0, 0); else glColor3f(0, 0, 0);
    desenhaTexto(-5, 5, "Iniciar Jogo");

    if (opcaoMenu == 1) glColor3f(1, 0, 0); else glColor3f(0, 0, 0);
    desenhaTexto(-5, 2, "Controles");

    if (opcaoMenu == 2) glColor3f(1, 0, 0); else glColor3f(0, 0, 0);
    desenhaTexto(-5, -1, "Sobre");

    glColor3f(0.5, 0.5, 0.5);
    desenhaTexto(-20, -15, "Use as setas para navegar e Enter para selecionar.");
}

void desenhaSobre() {
    glColor3f(0, 0, 0);
    desenhaTexto(-22, 15, "Jogo criado como projeto final da disciplina de");
    desenhaTexto(-22, 12, "Computacao Grafica, ofertada pela UNIVASF");
    desenhaTexto(-22, 9, "campus Juazeiro e ministrada pelo professor");
    desenhaTexto(-22, 6, "Jorge Cavalcanti.");
    
    desenhaTexto(-15, 0, "Desenvolvedores:");
    desenhaTexto(-13, -3, "Davi Cavalcanti");
    desenhaTexto(-13, -6, "Joison Junior");
    desenhaTexto(-13, -9, "Felipe Vieira");

    glColor3f(1, 0, 0);
    desenhaTexto(-15, -20, "Pressione 'Home' para voltar ao menu.");
}

void desenhaControles() {
    glColor3f(0, 0, 0);
    desenhaTexto(-8, 20, "CONTROLES");

    desenhaTexto(-15, 15, "Q - Lixeira Vermelha (Plastico)");
    desenhaTexto(-15, 12, "W - Lixeira Verde (Vidro)");
    desenhaTexto(-15, 9, "E - Lixeira Azul (Papel)");
    desenhaTexto(-15, 6, "R - Lixeira Amarela (Metal)");

    desenhaTexto(-15, 1, "Mouse - Mover a lixeira");

    desenhaTexto(-15, -4, "PgUp - Reiniciar o jogo (durante a partida)");
    desenhaTexto(-15, -7, "Home - Voltar ao menu principal");
    desenhaTexto(-15, -10, "End - Fechar o jogo");

    glColor3f(1, 0, 0);
    desenhaTexto(-15, -20, "Pressione 'Home' para voltar ao menu.");
}

// =================================================================
// MODIFICADO: Função 'Desenha' original renomeada para 'desenhaJogo'
// =================================================================
void desenhaJogo() {
    desenhaChao();
    desenhaLixos();

    glPushMatrix();
    glTranslatef(xLata, 0, 0);
    desenhaLata();
    glPopMatrix();

    char texto[50];
    glColor3f(0, 0, 0);
    sprintf(texto, "Pontos: %d", pontuacao);
    desenhaTexto(-wid + 1, hei - 27, texto);
    sprintf(texto, "Vidas: %d", vidas);
    desenhaTexto(-wid + 1, hei - 30, texto);

    if (vidas <= 0) {
        glColor3f(0, 0, 0);
        desenhaTexto(-10, 10, "FIM DE JOGO");
        desenhaTexto(-10, 7, "Pressione End para sair");
        desenhaTexto(-10, 4, "Pressione Home para voltar ao menu principal");
        desenhaTexto(-10, 1, "Pressione PgUp para reiniciar");
    }
}

// =================================================================
// MODIFICADO: Função principal de desenho que gerencia os estados
// =================================================================
void Desenha() {
    glMatrixMode(GL_MODELVIEW);
    glLoadIdentity();
    glClear(GL_COLOR_BUFFER_BIT);

    switch(estadoJogo) {
        case MENU:
            desenhaMenu();
            break;
        case JOGO:
            desenhaJogo();
            break;
        case CONTROLES:
            desenhaControles();
            break;
        case SOBRE:
            desenhaSobre();
            break;
    }
    glFlush();
}

// =================================================================
// MODIFICADO: Timer só executa a lógica do jogo se estiver no estado JOGO
// =================================================================
void Timer(int value) {
    if (estadoJogo == JOGO) {
        if (vidas > 0) {
            int tempoAtual = glutGet(GLUT_ELAPSED_TIME);

            if (tempoAtual - ultimoSpawn > intervalo) {
                novoLixo();
                ultimoSpawn = tempoAtual;
            }

            velocidadeLixo = 0.2 + (pontuacao / 100) * 0.05;
            if (pontuacao >= ultimoBonus + INTERVALO_BONUS) {
                ultimoBonus = pontuacao;
                vidas += BONUS_VIDAS;
            }
            atualizaLixos();
        }
    }
    glutPostRedisplay();
    glutTimerFunc(16, Timer, 0);
}

void AlteraJanela(GLsizei w, GLsizei h) {
    if (h == 0) h = 1;
    windowWidth = w;
    glViewport(0, 0, w, h);
    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    if (w <= h) {
        gluOrtho2D(-25.0f, 25.0f, -25.0f * h / w, 40.0f * h / w);
        wid = 25.0f;
        hei = 40.0f * h / w - (-25.0f * h / w);
    } else {
        gluOrtho2D(-25.0f * w / h, 25.0f * w / h, -25.0f, 40.0f);
        wid = 25.0f * w / h;
        hei = 40.0f - (-25.0f);
    }
}

// =================================================================
// NOVO: Função para resetar o estado do jogo
// =================================================================
void resetarJogo() {
    numLixos = 0;
    vidas = VIDAS_ORIGINAIS;
    pontuacao = 0;
    ultimoBonus = 0;
    velocidadeLixo = 0.2;
    ultimoSpawn = glutGet(GLUT_ELAPSED_TIME);
    xLata = 0;
    rLata = 1; gLata = 0; bLata = 0; estadosLata = 0;
}

// =================================================================
// MODIFICADO: Funções de input para gerenciar os estados
// =================================================================
void TeclasEspeciais(int key, int x, int y) {
    if (key == GLUT_KEY_END) exit(0);
    if (key == GLUT_KEY_HOME) {
        estadoJogo = MENU;
        opcaoMenu = 0;
    }

    switch (estadoJogo) {
        case MENU:
            if (key == GLUT_KEY_UP) opcaoMenu = (opcaoMenu - 1 + 3) % 3;
            else if (key == GLUT_KEY_DOWN) opcaoMenu = (opcaoMenu + 1) % 3;
            break;
        case JOGO:
            if (key == GLUT_KEY_PAGE_UP) resetarJogo();
            break;
        case CONTROLES: case SOBRE: break;
    }
}

void Teclado(unsigned char key, int x, int y) {
    switch (estadoJogo) {
        case MENU:
            if (key == 13) { // 13 = Enter
                 switch(opcaoMenu) {
                    case 0: resetarJogo(); estadoJogo = JOGO; break;
                    case 1: estadoJogo = CONTROLES; break;
                    case 2: estadoJogo = SOBRE; break;
                 }
            }
            break;
        case JOGO:
            switch (key) {
                case 'q': case 'Q': rLata = 1; gLata = bLata = 0; estadosLata = 0; break;
                case 'w': case 'W': gLata = 0.5; rLata = bLata = 0; estadosLata = 1; break;
                case 'e': case 'E': bLata = 1; rLata = gLata = 0; estadosLata = 2; break;
                case 'r': case 'R': rLata = gLata = 1; bLata = 0; estadosLata = 3; break;
            }
            break;
        case CONTROLES: case SOBRE: break;
    }
}

void controlaMouse(int button, int state, int x, int y) {
    if (estadoJogo == JOGO) {
        if ((button == GLUT_LEFT_BUTTON || button == GLUT_RIGHT_BUTTON) && state == GLUT_DOWN) {
            xLata = converteXMouse(x);
        }
    }
}

void arrastaMouse(int x, int y) {
    if (estadoJogo == JOGO) xLata = converteXMouse(x);
}

void Inicializa() {
    glClearColor(1, 1, 1, 1);
    srand(time(NULL));
}

int main(int argc, char **argv) {
    glutInit(&argc, argv);
    glutInitDisplayMode(GLUT_SINGLE | GLUT_RGB);
    glutInitWindowSize(600, 700);
    glutInitWindowPosition(5, 5);
    glutCreateWindow("Recycle Rush - Projeto Comp. Grafica");

    glutDisplayFunc(Desenha);
    glutReshapeFunc(AlteraJanela);
    glutKeyboardFunc(Teclado);
    glutSpecialFunc(TeclasEspeciais);
    glutMouseFunc(controlaMouse);
    glutMotionFunc(arrastaMouse);
    glutTimerFunc(0, Timer, 0);

    Inicializa();
    glutMainLoop();
    return 0;
}