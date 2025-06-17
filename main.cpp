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

typedef enum { MENU, JOGO, PAUSADO, CONTROLES, SOBRE } GameState;
GameState estadoJogo = MENU;
int opcaoMenu = 0;
int opcaoPausa = 0;

float velocidadeLixo = 0.2;

typedef struct {
    GLfloat x, y;
    GLfloat r, g, b;
    int tipo;
} Lixo;

GLfloat wid = 25, hei = 25;
GLint estadosLata = 0;
GLfloat rLata = 1, gLata = 0, bLata = 0;
GLfloat xLata = 0;

GLint windowWidth = 500;
GLint windowHeight = 700;
GLfloat orthoLeft, orthoRight, orthoTop, orthoBottom;

GLint intervalo = 1000;

Lixo lixos[MAX_LIXOS];
int numLixos = 0;
int ultimoSpawn = 0;
int pontuacao = 0;
int vidas = VIDAS_ORIGINAIS;
int ultimoBonus = 0;

int randInt(int max) {
    return rand() % max;
}

void geraCorLixo(GLfloat *r, GLfloat *g, GLfloat *b, int *tipo) {
    *tipo = randInt(TYPES);
    switch (*tipo) {
        case 0: *r = 1; *g = 0; *b = 0; break;
        case 1: *r = 0; *g = 0.5; *b = 0; break;
        case 2: *r = 0; *g = 0; *b = 1; break;
        case 3: *r = 1; *g = 1; *b = 0; break;
    }
}

void novoLixo() {
    if (numLixos < MAX_LIXOS) {
        int randX = randInt((int)(2 * wid));
        lixos[numLixos].x = -wid + (randX ? (randX == wid ? randX - 3 : randX) : 3);
        lixos[numLixos].y = hei - 1;
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

void desenhaPlastico() {
    glColor3f(1.0f, 0.2f, 0.2f);
    glBegin(GL_QUADS); glVertex2f(-1.0f, -2.0f); glVertex2f(1.0f, -2.0f); glVertex2f(1.0f, 0.5f); glVertex2f(-1.0f, 0.5f); glEnd();
    glBegin(GL_TRIANGLES); glVertex2f(-1.0f, 0.5f); glVertex2f(1.0f, 0.5f); glVertex2f(0.5f, 1.5f); glVertex2f(-1.0f, 0.5f); glVertex2f(-0.5f, 1.5f); glVertex2f(0.5f, 1.5f); glEnd();
    glColor3f(0.8f, 0.0f, 0.0f);
    glBegin(GL_QUADS); glVertex2f(-0.4f, 1.5f); glVertex2f(0.4f, 1.5f); glVertex2f(0.4f, 1.9f); glVertex2f(-0.4f, 1.9f); glEnd();
}

void desenhaVidro() {
    glColor3f(0.1f, 0.4f, 0.1f);
    glBegin(GL_TRIANGLES); glVertex2f(-1.5f, -1.5f); glVertex2f(0.0f, -1.0f); glVertex2f(-1.0f, 0.5f); glVertex2f(0.2f, -2.0f); glVertex2f(1.8f, -1.8f); glVertex2f(1.5f, 0.0f); glVertex2f(-0.5f, 0.8f); glVertex2f(1.0f, 1.2f); glVertex2f(0.0f, 2.0f); glEnd();
}

void desenhaPapel() {
    glColor3f(0.5f, 0.5f, 1.0f);
    glBegin(GL_POLYGON); glVertex2f(-1.5f, -1.0f); glVertex2f(0.0f, -1.8f); glVertex2f(1.5f, -0.5f); glVertex2f(1.2f, 1.0f); glVertex2f(0.0f, 1.5f); glVertex2f(-1.2f, 1.2f); glEnd();
    glColor3f(0.2f, 0.2f, 0.8f);
    glLineWidth(2);
    glBegin(GL_LINE_LOOP); glVertex2f(-1.5f, -1.0f); glVertex2f(0.0f, -1.8f); glVertex2f(1.5f, -0.5f); glVertex2f(1.2f, 1.0f); glVertex2f(0.0f, 1.5f); glVertex2f(-1.2f, 1.2f); glEnd();
}

void desenhaMetal() {
    glColor3f(1.0f, 1.0f, 0.0f);
    glBegin(GL_QUADS); glVertex2f(-1.2f, -1.5f); glVertex2f(1.2f, -1.5f); glVertex2f(1.2f, 1.5f); glVertex2f(-1.2f, 1.5f); glEnd();
    glColor3f(0.7f, 0.7f, 0.7f);
    glBegin(GL_QUADS); glVertex2f(-1.25f, -0.5f); glVertex2f(1.25f, -0.5f); glVertex2f(1.25f, 0.5f); glVertex2f(-1.25f, 0.5f); glEnd();
    glColor3f(0.6f, 0.6f, 0.6f);
    glBegin(GL_QUADS); glVertex2f(-1.2f, 1.5f); glVertex2f(1.2f, 1.5f); glVertex2f(1.2f, 1.7f); glVertex2f(-1.2f, 1.7f); glVertex2f(-1.2f, -1.7f); glVertex2f(1.2f, -1.7f); glVertex2f(1.2f, -1.5f); glVertex2f(-1.2f, -1.5f); glEnd();
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

void converteCoordenadasMouse(int x, int y, GLfloat* wx, GLfloat* wy) {
    *wx = orthoLeft + ((GLfloat)x / windowWidth) * (orthoRight - orthoLeft);
    *wy = orthoTop - ((GLfloat)y / windowHeight) * (orthoTop - orthoBottom);
}

void desenhaLata() {
    glColor3f(rLata, gLata, bLata);
    glBegin(GL_QUADS); glVertex2f(-3, -19.8); glVertex2f(3, -19.8); glVertex2f(3, -12); glVertex2f(-3, -12); glEnd();
}

void desenhaChao() {
    glColor3f(0, 1, 0);
    glLineWidth(3);
    glBegin(GL_LINES); glVertex2f(-wid, -20); glVertex2f(wid, -20); glEnd();
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

void desenhaMenu() {
    glColor3f(0, 0, 0);
    desenhaTexto(-8, 15, "RECYCLE RUSH");
    if (opcaoMenu == 0) glColor3f(1, 0, 0); else glColor3f(0, 0, 0);
    desenhaTexto(-5, 5, "Iniciar Jogo");
    if (opcaoMenu == 1) glColor3f(1, 0, 0); else glColor3f(0, 0, 0);
    desenhaTexto(-5, 2, "Controles");
    if (opcaoMenu == 2) glColor3f(1, 0, 0); else glColor3f(0, 0, 0);
    desenhaTexto(-5, -1, "Sobre");
    glColor3f(0.5, 0.5, 0.5);
    // MODIFICADO: Corrigida a grafia de "opção"
    desenhaTexto(-15, -15, "Use as setas ou o mouse para escolher.");
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
    desenhaTexto(-15, -13, "ESC - Pausar o jogo");
    glColor3f(1, 0, 0);
    desenhaTexto(-15, -20, "Pressione 'Home' para voltar ao menu.");
}

// =================================================================
// MODIFICADO: Cor de destaque do menu de pausa alterada para vermelho
// =================================================================
void desenhaPausa() {
    glColor4f(0.0f, 0.0f, 0.0f, 0.7f);
    glBegin(GL_QUADS); glVertex2f(-wid, -hei); glVertex2f(wid, -hei); glVertex2f(wid, hei); glVertex2f(-wid, hei); glEnd();
    glColor3f(1.0f, 1.0f, 1.0f);
    desenhaTexto(-5, 10, "PAUSADO");
    if (opcaoPausa == 0) glColor3f(1, 0, 0); else glColor3f(1, 1, 1);
    desenhaTexto(-8, 0, "Voltar ao Jogo");
    if (opcaoPausa == 1) glColor3f(1, 0, 0); else glColor3f(1, 1, 1);
    desenhaTexto(-8, -4, "Voltar para o Menu Inicial");
}

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

void Desenha() {
    glMatrixMode(GL_MODELVIEW);
    glLoadIdentity();
    glClear(GL_COLOR_BUFFER_BIT);
    switch (estadoJogo) {
        case MENU:      desenhaMenu(); break;
        case JOGO:      desenhaJogo(); break;
        case PAUSADO:   desenhaJogo(); desenhaPausa(); break;
        case CONTROLES: desenhaControles(); break;
        case SOBRE:     desenhaSobre(); break;
    }
    glFlush();
}

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
    windowHeight = h;
    glViewport(0, 0, w, h);
    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    if (w <= h) {
        orthoLeft = -25.0f; orthoRight = 25.0f;
        orthoBottom = -25.0f * h / w; orthoTop = 40.0f * h / w;
        gluOrtho2D(orthoLeft, orthoRight, orthoBottom, orthoTop);
        wid = 25.0f;
        hei = orthoTop - orthoBottom;
    } else {
        orthoLeft = -25.0f * w / h; orthoRight = 25.0f * w / h;
        orthoBottom = -25.0f; orthoTop = 40.0f;
        gluOrtho2D(orthoLeft, orthoRight, orthoBottom, orthoTop);
        wid = orthoRight;
        hei = orthoTop - orthoBottom;
    }
}

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
        case PAUSADO:
            if (key == GLUT_KEY_UP) opcaoPausa = (opcaoPausa - 1 + 2) % 2;
            else if (key == GLUT_KEY_DOWN) opcaoPausa = (opcaoPausa + 1) % 2;
            break;
        case CONTROLES: case SOBRE: break;
    }
    glutPostRedisplay();
}

void Teclado(unsigned char key, int x, int y) {
    switch (estadoJogo) {
        case MENU:
            if (key == 13) {
                 switch(opcaoMenu) {
                    case 0: resetarJogo(); estadoJogo = JOGO; break;
                    case 1: estadoJogo = CONTROLES; break;
                    case 2: estadoJogo = SOBRE; break;
                 }
            }
            break;
        case JOGO:
             if (key == 27) { estadoJogo = PAUSADO; opcaoPausa = 0; }
            switch (key) {
                case 'q': case 'Q': rLata = 1; gLata = bLata = 0; estadosLata = 0; break;
                case 'w': case 'W': gLata = 0.5; rLata = bLata = 0; estadosLata = 1; break;
                case 'e': case 'E': bLata = 1; rLata = gLata = 0; estadosLata = 2; break;
                case 'r': case 'R': rLata = gLata = 1; bLata = 0; estadosLata = 3; break;
            }
            break;
        case PAUSADO:
            if (key == 27) estadoJogo = JOGO;
            if (key == 13) {
                if(opcaoPausa == 0) estadoJogo = JOGO;
                else { estadoJogo = MENU; opcaoMenu = 0; }
            }
            break;
        case CONTROLES: case SOBRE: break;
    }
    glutPostRedisplay();
}

void controlaMouse(int button, int state, int x, int y) {
    if (button == GLUT_LEFT_BUTTON && state == GLUT_DOWN) {
        GLfloat mouseX, mouseY;
        converteCoordenadasMouse(x, y, &mouseX, &mouseY);
        switch(estadoJogo) {
            case MENU:
                if (mouseX > -10 && mouseX < 10 && mouseY > 4 && mouseY < 7) {
                    resetarJogo(); estadoJogo = JOGO;
                }
                if (mouseX > -10 && mouseX < 10 && mouseY > 1 && mouseY < 4) {
                    estadoJogo = CONTROLES;
                }
                if (mouseX > -10 && mouseX < 10 && mouseY < 1 && mouseY > -2) {
                    estadoJogo = SOBRE;
                }
                break;
            case PAUSADO:
                if (mouseX > -12 && mouseX < 12 && mouseY > -1 && mouseY < 3) {
                    estadoJogo = JOGO;
                }
                if (mouseX > -12 && mouseX < 12 && mouseY > -5 && mouseY < -2) {
                    estadoJogo = MENU; opcaoMenu = 0;
                }
                break;
            case JOGO: {
                GLfloat limiteEsquerdo = -wid + 3, limiteDireito = wid - 3;
                if (mouseX < limiteEsquerdo) xLata = limiteEsquerdo;
                else if (mouseX > limiteDireito) xLata = limiteDireito;
                else xLata = mouseX;
                break;
            }
            default: break;
        }
        glutPostRedisplay();
    }
}

void arrastaMouse(int x, int y) {
    if (estadoJogo == JOGO) {
        GLfloat mouseX, mouseY;
        converteCoordenadasMouse(x, y, &mouseX, &mouseY);
        GLfloat limiteEsquerdo = -wid + 3, limiteDireito = wid - 3;
        if (mouseX < limiteEsquerdo) xLata = limiteEsquerdo;
        else if (mouseX > limiteDireito) xLata = limiteDireito;
        else xLata = mouseX;
        glutPostRedisplay();
    }
}

// =================================================================
// NOVO: Função para detectar movimento do mouse sem clique (hover)
// =================================================================
void passiveMotion(int x, int y){
    GLfloat mouseX, mouseY;
    converteCoordenadasMouse(x, y, &mouseX, &mouseY);

    switch(estadoJogo){
        case MENU: {
            int novaOpcao = -1;
            if (mouseX > -10 && mouseX < 10 && mouseY > 4 && mouseY < 7) novaOpcao = 0;
            else if (mouseX > -10 && mouseX < 10 && mouseY > 1 && mouseY < 4) novaOpcao = 1;
            else if (mouseX > -10 && mouseX < 10 && mouseY < 1 && mouseY > -2) novaOpcao = 2;
            
            if(novaOpcao != -1 && opcaoMenu != novaOpcao){
                opcaoMenu = novaOpcao;
                glutPostRedisplay();
            }
            break;
        }
        case PAUSADO: {
            int novaOpcao = -1;
            if (mouseX > -12 && mouseX < 12 && mouseY > -1 && mouseY < 3) novaOpcao = 0;
            else if (mouseX > -12 && mouseX < 12 && mouseY > -5 && mouseY < -2) novaOpcao = 1;

            if(novaOpcao != -1 && opcaoPausa != novaOpcao){
                opcaoPausa = novaOpcao;
                glutPostRedisplay();
            }
            break;
        }
        default: break;
    }
}


void Inicializa() {
    glClearColor(1, 1, 1, 1);
    srand(time(NULL));
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
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
    // =================================================================
    // NOVO: Registro da função de movimento passivo do mouse
    // =================================================================
    glutPassiveMotionFunc(passiveMotion);
    glutTimerFunc(0, Timer, 0);
    Inicializa();
    glutMainLoop();
    return 0;
}