#include <3ds.h>
#include <citro2d.h>
#include <stdio.h>
#include <string.h>
#include <ctype.h>

#define MAX_WORD_LEN 32
#define MAX_ERRORS 6

typedef enum {
    STATE_MENU,
    STATE_INPUT_WORD,
    STATE_PLAYING,
    STATE_GAME_OVER,
    STATE_VICTORY
} GameState;

typedef enum {
    LANG_EN,
    LANG_FR
} Language;

GameState currentState = STATE_MENU;
Language currentLang = LANG_EN;

char secretWord[MAX_WORD_LEN] = "";
char guessedWord[MAX_WORD_LEN] = "";
char usedLetters[26] = {0};
int usedLettersCount = 0;
int errorsCount = 0;

const char* getText(const char* key) {
    if (currentLang == LANG_FR) {
        if (strcmp(key, "TITLE") == 0) return "HANGMAN GAME";
        if (strcmp(key, "START") == 0) return "Appuyez sur A pour commencer";
        if (strcmp(key, "LANG_TOGGLE") == 0) return "Appuyez sur Y pour changer la langue";
        if (strcmp(key, "PROMPT_WORD") == 0) return "Joueur 1 : Entrez un mot secret";
        if (strcmp(key, "GUESS_LETTER") == 0) return "Joueur 2 : Devinez une lettre !";
        if (strcmp(key, "TOUCH_HINT") == 0) return "Touchez l'ecran du bas pour proposer 1 lettre";
        if (strcmp(key, "HP") == 0) return "Vie restante :";
        if (strcmp(key, "USED_LETTERS") == 0) return "Lettres utilisees :";
        if (strcmp(key, "WIN") == 0) return "VICTOIRE ! Le Joueur 2 a trouve !";
        if (strcmp(key, "LOSE") == 0) return "DEFAITE ! Le Joueur 1 a gagne !";
        if (strcmp(key, "SECRET_WAS") == 0) return "Le mot etait :";
        if (strcmp(key, "RESTART") == 0) return "Appuyez sur A pour rejouer";
    } else {
        if (strcmp(key, "TITLE") == 0) return "HANGMAN GAME";
        if (strcmp(key, "START") == 0) return "Press A to Start";
        if (strcmp(key, "LANG_TOGGLE") == 0) return "Press Y to change language";
        if (strcmp(key, "PROMPT_WORD") == 0) return "Player 1: Enter a secret word";
        if (strcmp(key, "GUESS_LETTER") == 0) return "Player 2: Guess a letter!";
        if (strcmp(key, "TOUCH_HINT") == 0) return "Touch bottom screen to guess a letter";
        if (strcmp(key, "HP") == 0) return "Remaining HP:";
        if (strcmp(key, "USED_LETTERS") == 0) return "Used letters:";
        if (strcmp(key, "WIN") == 0) return "VICTORY! Player 2 guessed it!";
        if (strcmp(key, "LOSE") == 0) return "GAME OVER! Player 1 wins!";
        if (strcmp(key, "SECRET_WAS") == 0) return "The word was:";
        if (strcmp(key, "RESTART") == 0) return "Press A to play again";
    }
    return key;
}

void openSoftwareKeyboard() {
    SwkbdState swkbd;
    char mybuf[MAX_WORD_LEN];
    swkbdInit(&swkbd, SWKBD_TYPE_WESTERN, 2, MAX_WORD_LEN - 1);
    swkbdSetValidation(&swkbd, SWKBD_NOTEMPTY_NOTBLANK, 0, 0);
    swkbdSetHintText(&swkbd, currentLang == LANG_FR ? "Entrez le mot secret" : "Enter secret word");
    
    SwkbdButton button = swkbdInputText(&swkbd, mybuf, sizeof(mybuf));

    if (button == SWKBD_BUTTON_CONFIRM) {
        int j = 0;
        for (int i = 0; mybuf[i] != '\0'; i++) {
            if (isalpha((unsigned char)mybuf[i])) {
                secretWord[j] = toupper((unsigned char)mybuf[i]);
                guessedWord[j] = '_';
                j++;
            }
        }
        secretWord[j] = '\0';
        guessedWord[j] = '\0';

        if (strlen(secretWord) > 0) {
            errorsCount = 0;
            usedLettersCount = 0;
            memset(usedLetters, 0, sizeof(usedLetters));
            currentState = STATE_PLAYING;
        }
    }
}

void guessLetter(char c) {
    c = toupper((unsigned char)c);
    
    for (int i = 0; i < usedLettersCount; i++) {
        if (usedLetters[i] == c) return;
    }
    
    usedLetters[usedLettersCount++] = c;
    
    int found = 0;
    for (int i = 0; secretWord[i] != '\0'; i++) {
        if (secretWord[i] == c) {
            guessedWord[i] = c;
            found = 1;
        }
    }
    
    if (!found) {
        errorsCount++;
        if (errorsCount >= MAX_ERRORS) {
            currentState = STATE_GAME_OVER;
        }
    } else {
        if (strcmp(secretWord, guessedWord) == 0) {
            currentState = STATE_VICTORY;
        }
    }
}

int main(int argc, char* argv[]) {
    gfxInitDefault();
    C3D_Init(C3D_DEFAULT_CMDBUF_SIZE);
    C2D_Init(C2D_DEFAULT_MAX_OBJECTS);
    C2D_Prepare();
    
    consoleInit(GFX_BOTTOM, NULL);

    C3D_RenderTarget* top = C2D_CreateScreenTarget(GFX_TOP, GFX_LEFT);
    u32 clrDark  = C2D_Color32(30, 30, 30, 255);

    C2D_TextBuf textBuf = C2D_TextBufNew(4096);

    while (aptMainLoop()) {
        hidScanInput();
        u32 kDown = hidKeysDown();

        if (kDown & KEY_START) break;

        if (currentState == STATE_MENU) {
            if (kDown & KEY_A) currentState = STATE_INPUT_WORD;
            if (kDown & KEY_Y) currentLang = (currentLang == LANG_EN) ? LANG_FR : LANG_EN;
        } 
        else if (currentState == STATE_INPUT_WORD) {
            openSoftwareKeyboard();
        } 
        else if (currentState == STATE_PLAYING) {
            if (kDown & KEY_TOUCH) {
                SwkbdState swkbd;
                char buf[4];
                swkbdInit(&swkbd, SWKBD_TYPE_WESTERN, 1, 1);
                swkbdSetHintText(&swkbd, currentLang == LANG_FR ? "Lettre" : "Letter");
                if (swkbdInputText(&swkbd, buf, sizeof(buf)) == SWKBD_BUTTON_CONFIRM) {
                    if (isalpha((unsigned char)buf[0])) {
                        guessLetter(buf[0]);
                    }
                }
            }
        } 
        else if (currentState == STATE_GAME_OVER || currentState == STATE_VICTORY) {
            if (kDown & KEY_A) currentState = STATE_MENU;
        }

        C3D_FrameBegin(C3D_FRAME_SYNCDRAW);
        C2D_TargetClear(top, clrDark);
        C2D_SceneBegin(top);
        C2D_TextBufClear(textBuf);

        // Affichage du pourcentage de vie sur l'écran du haut
        if (currentState == STATE_PLAYING || currentState == STATE_VICTORY || currentState == STATE_GAME_OVER) {
            int hp = 100 - ((errorsCount * 100) / MAX_ERRORS);
            if (hp < 0) hp = 0;
            
            char hpStr[64];
            snprintf(hpStr, sizeof(hpStr), "VIE / HP : %d%%", hp);
            
            C2D_Text hpText;
            C2D_TextParse(&hpText, textBuf, hpStr);
            C2D_TextOptimize(&hpText);
            C2D_DrawText(&hpText, C2D_WithColor, 50.0f, 100.0f, 0.5f, 0.8f, 0.8f, C2D_Color32(255, 255, 255, 255));
        }

        C3D_FrameEnd(0);

        consoleClear();
        printf("\x1b[1;1H=== %s ===", getText("TITLE"));
        printf("\x1b[3;1HLangue / Language: %s", (currentLang == LANG_FR ? "Francais" : "English"));
        printf("\x1b[29;1H[START] Quitter / Exit");

        if (currentState == STATE_MENU) {
            printf("\x1b[10;1H%s", getText("START"));
            printf("\x1b[12;1H%s", getText("LANG_TOGGLE"));
        } 
        else if (currentState == STATE_PLAYING) {
            printf("\x1b[6;1H%s", getText("GUESS_LETTER"));
            printf("\x1b[8;1H%s", getText("TOUCH_HINT"));
            printf("\x1b[12;1HMot : ");
            for (size_t i = 0; i < strlen(guessedWord); i++) {
                printf("%c ", guessedWord[i]);
            }
            printf("\n\n%s %d%%", getText("HP"), (100 - ((errorsCount * 100) / MAX_ERRORS)));
            printf("\n%s ", getText("USED_LETTERS"));
            for (int i = 0; i < usedLettersCount; i++) {
                printf("%c ", usedLetters[i]);
            }
        } 
        else if (currentState == STATE_VICTORY) {
            printf("\x1b[10;1H%s", getText("WIN"));
            printf("\x1b[12;1H%s %s", getText("SECRET_WAS"), secretWord);
            printf("\x1b[15;1H%s", getText("RESTART"));
        } 
        else if (currentState == STATE_GAME_OVER) {
            printf("\x1b[10;1H%s", getText("LOSE"));
            printf("\x1b[12;1H%s %s", getText("SECRET_WAS"), secretWord);
            printf("\x1b[15;1H%s", getText("RESTART"));
        }
    }

    C2D_TextBufDelete(textBuf);
    C2D_Fini();
    C3D_Fini();
    gfxExit();
    return 0;
}
