#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <GL/freeglut.h>
#include <string.h> // Para sprintf

#define TYPES 4
#define MAX_LIXOS 12
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
int vidas = 0; // Número de vidas do jogador

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

void atualizaLixos() {
    for (int i = 0; i < numLixos; i++) {
        lixos[i].y -= velocidadeLixo;
        
        // Verifica se o lixo atingiu a lixeira
        if (lixos[i].y < -12 && lixos[i].y > -20) 
        {
            if (lixos[i].x > xLata - 3 && lixos[i].x < xLata + 3) 
            {
                // Verifica se a cor do lixo corresponde à cor da lixeira
                if (lixos[i].tipo == estadosLata) 
                    pontuacao += 10; // Acertou a cor
                else
                    vidas--; // Errou a cor
                
                // Remove o lixo
                for (int j = i; j < numLixos - 1; j++) {
                    lixos[j] = lixos[j + 1];
                }
                numLixos--;
                i--;
                continue;
            }
        }
        
        // Remove lixos que caíram do chão
        if (lixos[i].y < -20) 
        {
            vidas--; // Perde uma vida por lixo que caiu
            for (int j = i; j < numLixos - 1; j++) 
                lixos[j] = lixos[j + 1];
            numLixos--;
            i--;
        }
    }
}

GLfloat converteXMouse(int x) {
    GLfloat xConvertido = -wid + (2 * wid * (GLfloat)x / windowWidth);
    GLfloat limiteEsquerdo = -wid + 3;
    GLfloat limiteDireito = wid - 3;
    
    if (xConvertido < limiteEsquerdo) xConvertido = limiteEsquerdo;
    else if (xConvertido > limiteDireito) xConvertido = limiteDireito;
    
    return xConvertido;
}

void desenhaLata() {
    glColor3f(rLata, gLata, bLata);
    glBegin(GL_QUADS);
        glVertex2f(-3, -19.8);
        glVertex2f(3, -19.8);
        glVertex2f(3, -12);
        glVertex2f(-3, -12);
    glEnd();
}

void desenhaChao() {
    glColor3f(0, 1, 0);
    glLineWidth(3);
    glBegin(GL_LINES);
        glVertex2f(-wid, -20);
        glVertex2f(wid, -20);
    glEnd();
}

void desenhaLixos() {
    for (int i = 0; i < numLixos; i++) {
        glPushMatrix();
        glTranslatef(lixos[i].x, lixos[i].y, 0);
        glColor3f(lixos[i].r, lixos[i].g, lixos[i].b);
        glBegin(GL_QUADS);
            glVertex2f(-2, 0);
            glVertex2f(2, 0);
            glVertex2f(2, -2);
            glVertex2f(-2, -2);
        glEnd();
        glPopMatrix();
    }
}

void desenhaEixos() {
    glColor3f(0, 0, 0);
    glBegin(GL_LINES);
        glVertex2f(-wid, 0);
        glVertex2f(wid, 0);
        glVertex2f(0, -hei);
        glVertex2f(0, hei);
    glEnd();
}

void Desenha() {
    glMatrixMode(GL_MODELVIEW);
    glLoadIdentity();
    glClear(GL_COLOR_BUFFER_BIT);

    //desenhaEixos();

    desenhaChao();
    desenhaLixos();

    glPushMatrix();
    glTranslatef(xLata, 0, 0);
    desenhaLata();
    glPopMatrix();

    // Desenha a pontuação
    glColor3f(0, 0, 0);
    char textoPontuacao[50];
    sprintf(textoPontuacao, "Pontos: %d", pontuacao);
    desenhaTexto(-wid + 1, hei - 27, textoPontuacao);

    // Desenha as vidas
    char textoVidas[50];
    sprintf(textoVidas, "Vidas: %d", vidas);
    desenhaTexto(-wid + 1, hei - 30, textoVidas);

    // Verifica se o jogo acabou, e se foi resetado, os textos somem
    if (vidas <= 0) {
        glColor3f(0, 0, 0);
        desenhaTexto(wid/2 - 1, hei/2, "FIM DE JOGO");
        desenhaTexto(-10, -2, "Pressione End para sair");
        desenhaTexto(-10, -5, "Pressione Home para voltar ao menu principal");
        desenhaTexto(-10, -8, "Pressione PgUp para reiniciar");
    }

    glFlush();
}

void Timer(int value) {
    if (vidas > 0) {
        int tempoAtual = glutGet(GLUT_ELAPSED_TIME);

        if (tempoAtual - ultimoSpawn > intervalo) {
            novoLixo();
            ultimoSpawn = tempoAtual;
        }

        // Aumenta a velocidade a cada 100 pontos
        velocidadeLixo = 0.2 + (pontuacao / 100) * 0.05;

        atualizaLixos();
        glutPostRedisplay();
    }
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
        hei = 40.0f * h / w - (-25.0f * h / w); // hei = top - bottom
    } else {
        gluOrtho2D(-25.0f * w / h, 25.0f * w / h, -25.0f, 40.0f);
        wid = 25.0f * w / h;
        hei = 40.0f - (-25.0f); // hei = top - bottom
    }
}

void TeclasEspeciais(int key, int x, int y) {
    switch (key) {
        // fecha o jogo
        case GLUT_KEY_END: {
            exit(0);
        }
        // retorna ao menu principal
        case GLUT_KEY_HOME: {
            break;
        }
        // reseta o jogo
        case GLUT_KEY_PAGE_UP: {
            // numLixos = 0 limpa todos os lixos da tela
            numLixos = 0;
            vidas = 3;
            pontuacao = 0;
            break;
        }
    }
}

void Teclado(unsigned char key, int x, int y) {
    switch (key) {
        case 'q': case 'Q': rLata = 1; gLata = bLata = 0; estadosLata = 0; break; // Plástico
        case 'w': case 'W': gLata = 0.5; rLata = bLata = 0; estadosLata = 1; break; // Vidro
        case 'e': case 'E': bLata = 1; rLata = gLata = 0; estadosLata = 2; break; // Papel
        case 'r': case 'R': rLata = gLata = 1; bLata = 0; estadosLata = 3; break; // Metal
    }
    glutPostRedisplay();
}

void controlaMouse(int button, int state, int x, int y) {
    if ((button == GLUT_LEFT_BUTTON || button == GLUT_RIGHT_BUTTON) && state == GLUT_DOWN) {
        xLata = converteXMouse(x);
        glutPostRedisplay();
    }
}

void arrastaMouse(int x, int y) {
    xLata = converteXMouse(x);
    glutPostRedisplay();
}

void Inicializa() {
    glClearColor(1, 1, 1, 1);
    srand(time(NULL));
    ultimoSpawn = glutGet(GLUT_ELAPSED_TIME);
}

void aumentarVelocidade(){
    
}

int main(int argc, char **argv) {
    glutInit(&argc, argv);
    glutInitDisplayMode(GLUT_SINGLE | GLUT_RGB);
    glutInitWindowSize(500, 500);
    glutInitWindowPosition(5, 5);
    glutCreateWindow("Projeto - Comp. Grafica");

    glutDisplayFunc(Desenha);
    glutReshapeFunc(AlteraJanela);
    glutKeyboardFunc(Teclado);
    glutSpecialFunc(TeclasEspeciais);
    glutMouseFunc(controlaMouse);
    glutMotionFunc(arrastaMouse);
    glutTimerFunc(0, Timer, 0);

    Inicializa();
    glutMainLoop();
}
