#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>

#define LEFT_PATH 0
#define RIGHT_PATH 1

const char *HELP = "Syntax: {argv[0]} <command> [R] [C] [FILE]\n"
                   "        {argv[0]} --help\n"
                   "   *<command> is mandatory but mutually exclusive\n" 
                   "Commands:\n"    
                   "       --test [FILE]: Testing allocating memory and validity of the file.\n"
                   "       --rpath [R] [C] [FILE]: Goes through the maze by holding a wall with the RIGHT hand, prints out the cells it went through\n"
                   "       --lpath [R] [C] [FILE]: Goes through the maze by holding a wall with the LEFT hand, prints out the cells it went through\n"
                   "Arguments:\n"
                   "       [R]: Row, in which you want to start solving the maze\n"
                   "       [C]: Column, in which you want to start solving the maze\n"
                   "       [File]: Address of the file, in which the maze is\n";

typedef struct
{
    int rows;
    int cols;
    unsigned char *cells;
} Map;

typedef enum
{
    DOPRAVA,
    HORE,
    DOLAVA,
    DOLE,
} POCIATOCNY_SMER;

typedef enum
{
    LAVA_HRANICA,
    PRAVA_HRANICA,
    HORNA_HRANICA,
    DOLNA_HRANICA,
} HRANICA;

int horna_dolna_hranica(int r, int c);
void binarny_rozklad(int cislo, int binarny_rozklad[3]);
bool is_border(Map *map, int r, int c, int border);

/**
 * @brief Uvolnenie dynamickej pamate
 * funkcia uvolni alokovanu pamat v celej mapa_bludiska
 * 
 * @param mapa_bludiska ukazatel na mapu bludiska v strukture Map
 */
void free_map(Map *mapa_bludiska)
{
    free(mapa_bludiska->cells);
    free(mapa_bludiska);
}

/**
 * @brief Inicializacia mapy (dynamicky alokovana pamat) do struktury Map
 * 
 * @param rad Pocet riadkov mapy
 * @param stlpec Pocet stlpcov mapy
 * 
 * @return Vracia ukazatel na strukturu Map s inicializovanou mapou 
 */
Map *inicialize_map(int rad, int stlpec)
{
    Map *mapa_bludiska = (Map*)malloc(sizeof(Map));
    mapa_bludiska->rows = rad;
    mapa_bludiska->cols = stlpec;
    mapa_bludiska->cells = (unsigned char*)malloc(rad * stlpec * sizeof(unsigned char));
    
    return mapa_bludiska;
}

/**
 * @brief Ziskanie mapy zo suboru
 * 
 * Vo funkcii inicializujeme mapu a testuje sa jej validita
 * Zapiseme ziskane udaje do struktury Map->cells
 * 
 * @param bludisko ukazatel na subor s bludiskom
 * @return mapu bludiska v strukture Map*
 */
Map *get_map(char *bludisko)
{
    FILE *subor_bludiska = fopen(bludisko, "r");
    if(subor_bludiska == NULL)
    {
        fprintf(stderr, "Invalid\n");
        return NULL;
    }

    int pocet_radov, pocet_stlpcov;
    if(fscanf(subor_bludiska, "%d %d", &pocet_radov, &pocet_stlpcov) != 2 || pocet_radov < 1 || pocet_stlpcov < 1)
    {
        fprintf(stderr, "Invalid\n");
        fclose(subor_bludiska);
        return NULL;
    }

    Map *mapa_bludiska = inicialize_map(pocet_radov, pocet_stlpcov);
    for(int i = 0; i < pocet_radov * pocet_stlpcov; i++)
    {
        if(fscanf(subor_bludiska, "%hhd", &mapa_bludiska->cells[i]) != 1)
        {
            fclose(subor_bludiska);
            free_map(mapa_bludiska);
            return NULL;
        }
        
        int cislo_zo_subora = mapa_bludiska->cells[i];
        int binarny_rozklad_cisla[3];
        binarny_rozklad(cislo_zo_subora, binarny_rozklad_cisla);
        if(cislo_zo_subora < 0 || cislo_zo_subora > 7)
        {
            fclose(subor_bludiska);
            free_map(mapa_bludiska);
            return NULL;
        }
        
        int momentalny_rad = i / pocet_stlpcov;
        int momentalny_stlpec = i % pocet_stlpcov;
        if(momentalny_stlpec > 0)
        {
            int cislo_zo_subora_nalavo = mapa_bludiska->cells[i - 1];
            int binarny_rozklad_cisla_nalavo[3];
            binarny_rozklad(cislo_zo_subora_nalavo, binarny_rozklad_cisla_nalavo);
            if(binarny_rozklad_cisla[0] != binarny_rozklad_cisla_nalavo[1])
            {
                fclose(subor_bludiska);
                free_map(mapa_bludiska);
                return NULL;
            }
        }
        if(momentalny_rad > 0)
        {
            int cislo_zo_subora_hore = mapa_bludiska->cells[i - pocet_stlpcov];
            int rad_cisla_hore = (i - pocet_stlpcov) / pocet_stlpcov; 
            int stlpec_cisla_hore = (i - pocet_stlpcov) % pocet_stlpcov;
            int binarny_rozklad_cisla_hore[3];
            binarny_rozklad(cislo_zo_subora_hore, binarny_rozklad_cisla_hore);
            if(horna_dolna_hranica(momentalny_rad, momentalny_stlpec) == HORNA_HRANICA && horna_dolna_hranica(rad_cisla_hore, stlpec_cisla_hore) == DOLNA_HRANICA)
            {
                if(binarny_rozklad_cisla[2] != binarny_rozklad_cisla_hore[2])
                {
                    fclose(subor_bludiska);
                    free_map(mapa_bludiska);
                    return NULL;
                }
            }
        }
    }
    fclose(subor_bludiska);
    return mapa_bludiska;
}

/**
 * @brief Binarny rozklad cisla
 * 
 * Funkcia rozklada cislo na 3 binarne cisla pomocou % a /
 * 
 * @param cislo cislo, ktore chceme rozlozit
 * @param binarny_rozklad pole, do ktoreho sa ulozia binarne cisla
 */
void binarny_rozklad(int cislo, int binarny_rozklad[3])
{
    for(int x = 0; x < 3; x++)
    {
        binarny_rozklad[x] = cislo % 2;
        cislo = cislo / 2;
    }
}

/**
 * @brief Zisti, ci sa na policku nachadza konkretna stena
 * 
 * pomocou binarneho rozkladu funkcia zisti pravdivostnu hodnotu steny
 * 
 * @param map ukazatel na mapu bludiska v strukture Map
 * @param r riadok policka
 * @param stlpec policka
 * @param border hranica, ktora sa kontroluje
 * 
 * @return pravdivostnu hodnotu konkretnej hranice
*/
bool is_border(Map *map, int r, int c, int border)
{
    int index = r * map->cols + c;
    int cislo_zo_subora = map->cells[index];
    int binarny_rozklad_cisla[3];
    binarny_rozklad(cislo_zo_subora, binarny_rozklad_cisla);

    switch (border)
    {
    case 0:
        return (binarny_rozklad_cisla[0] == 1);
    case 1:
        return (binarny_rozklad_cisla[1] == 1);
    case 2:
        return (binarny_rozklad_cisla[2] == 1);
    case 3:
        return (binarny_rozklad_cisla[2] == 1);
    default:
        return false;
    }
}

/**
 * @brief Zisti, ci ma policko hornu alebo dolnu hranicu
 * 
 * Podla toho, ci je sucet riadku a stlpca parny alebo neparny vieme otocenie policka
 * 
 * @param r riadok, v ktorom sa nachadza policko
 * @param c stlpec, v ktorom sa nachadza policko
 * 
 * @return Vracia hornu alebo dolnu hranicu
*/
int horna_dolna_hranica(int r, int c)
{
    if((r + c) % 2 == 0)
        return HORNA_HRANICA;
    else
        return DOLNA_HRANICA;
}

/**
 * @brief Obe funkcie zistia, kde sa nachadza pociatocna stena
 * 
 * Funkcie sa pozru na to, kde je vstup a podla toho urcia pociatocnu stenu
 * 
 * @param map ukazatel na mapu bludiska v strukture Map
 * @param r riadok, v ktorom sa nachadza policko
 * @param c stlpec, v ktorom sa nachadza policko
 * 
 * @return Vracia pociatocnu stenu po ktorej sa ma konkretny algoritmus vydat
*/
int start_border_right(Map *map, int r, int c)
{
    int horna_or_dolna_hranica = horna_dolna_hranica(r, c);
    if(r == 0)
    {
        if(c == 0)
        {
            if(is_border(map, r, c, LAVA_HRANICA) == true && is_border(map, r, c, HORNA_HRANICA) == false)
                return LAVA_HRANICA;
            else if( is_border(map, r, c, LAVA_HRANICA) == false)
                return DOLNA_HRANICA;
        }
        else if(c == map->cols - 1)
        {
            if(horna_or_dolna_hranica == HORNA_HRANICA)
            {
                if(is_border(map, r, c, HORNA_HRANICA) == true && is_border(map, r, c, PRAVA_HRANICA) == false)
                    return HORNA_HRANICA;
                else if(is_border(map, r, c, LAVA_HRANICA) == true && is_border(map, r, c, HORNA_HRANICA) == false)
                    return LAVA_HRANICA;
            }
            else if(horna_or_dolna_hranica == DOLNA_HRANICA && is_border(map, r, c, LAVA_HRANICA) == true && is_border(map, r, c, PRAVA_HRANICA) == false)
                return PRAVA_HRANICA;
            else if(horna_or_dolna_hranica == DOLNA_HRANICA && is_border(map, r, c, LAVA_HRANICA) == false && is_border(map, r, c, PRAVA_HRANICA) == false)
                return HORNA_HRANICA;
        }
        else if(0 < c && c < map->cols - 1 && horna_or_dolna_hranica == HORNA_HRANICA && is_border(map, r, c, HORNA_HRANICA) == false)
            return LAVA_HRANICA;
    }
    else if(r == map->rows - 1)
    {
        if(c == 0)
        {
            if(horna_or_dolna_hranica == DOLNA_HRANICA)
            {
                if(is_border(map, r, c, PRAVA_HRANICA) == true && is_border(map, r, c, DOLNA_HRANICA) == false)
                    return PRAVA_HRANICA;
                else if(is_border(map, r, c, DOLNA_HRANICA) == true && is_border(map, r, c, LAVA_HRANICA) == false)
                    return DOLNA_HRANICA;
            }
            else if(horna_or_dolna_hranica == HORNA_HRANICA && is_border(map, r, c, PRAVA_HRANICA) == true && is_border(map, r, c, LAVA_HRANICA) == false)
                return PRAVA_HRANICA;
            else if(horna_or_dolna_hranica == HORNA_HRANICA && is_border(map, r, c, PRAVA_HRANICA) == false && is_border(map, r, c, LAVA_HRANICA) == false)
                return DOLNA_HRANICA;
        }
        else if(c == map->cols - 1)
        {
            if(horna_or_dolna_hranica == DOLNA_HRANICA )
            {
                if(is_border(map, r, c, PRAVA_HRANICA) == true && is_border(map, r, c, DOLNA_HRANICA) == false)
                    return PRAVA_HRANICA;
                else if(is_border(map, r, c, LAVA_HRANICA) == true && is_border(map, r, c, PRAVA_HRANICA) == false)
                    return LAVA_HRANICA;
                else if(is_border(map, r, c, LAVA_HRANICA) == false && is_border(map, r, c, PRAVA_HRANICA) == false)
                    return HORNA_HRANICA;
            }
            else if(horna_or_dolna_hranica == HORNA_HRANICA && is_border(map, r, c, PRAVA_HRANICA) == false)
                return HORNA_HRANICA;
        }
        else if(c != 0 && c != map->cols - 1 && horna_or_dolna_hranica == DOLNA_HRANICA && is_border(map, r, c, DOLNA_HRANICA) == false)
            return PRAVA_HRANICA;
    }
    else if(c == 0 && 0 < r && r < map->rows - 1)
    {
        if(horna_or_dolna_hranica == DOLNA_HRANICA)
        {
            if(is_border(map, r, c, LAVA_HRANICA) == false)
                return DOLNA_HRANICA;
        }
        else
        {
            if(is_border(map, r, c, PRAVA_HRANICA) == true && is_border(map, r, c, LAVA_HRANICA) == false)
                return PRAVA_HRANICA;
            else if(is_border(map, r, c, PRAVA_HRANICA) == false && is_border(map, r, c, LAVA_HRANICA) == false)
                return DOLNA_HRANICA;
        }
    }
    else if(c == map->cols -1 && 0 < r && r < map->rows - 1)
    {
        if(horna_or_dolna_hranica == HORNA_HRANICA)
        {
            if(is_border(map, r, c, PRAVA_HRANICA) == false)
                return HORNA_HRANICA;
        }
        else
        {
            if(is_border(map, r, c, LAVA_HRANICA) == true && is_border(map, r, c, PRAVA_HRANICA) == false)
                return LAVA_HRANICA;
            else if(is_border(map, r, c, LAVA_HRANICA) == false && is_border(map, r, c, PRAVA_HRANICA) == false)
                return HORNA_HRANICA;
        }
    }
    return -1;
}

int start_border_left(Map *map, int r, int c)
{
    int horna_or_dolna_hranica = horna_dolna_hranica(r, c);
    if(r == 0)
    {
        if(c== 0)
        {
            if(is_border(map, r, c, HORNA_HRANICA) == true && is_border(map, r, c, LAVA_HRANICA) == false)
                return HORNA_HRANICA;
            else if(is_border(map, r, c, PRAVA_HRANICA) == true && is_border(map, r, c, HORNA_HRANICA) == false)
                return PRAVA_HRANICA;
        }
        else if(c == map->cols - 1)
        {
            if(horna_or_dolna_hranica == HORNA_HRANICA)
            {
                if(is_border(map, r, c, PRAVA_HRANICA) == true && is_border(map, r, c, HORNA_HRANICA) == false)
                    return PRAVA_HRANICA;
                 else if(is_border(map, r, c, LAVA_HRANICA) == false && is_border(map, r, c, PRAVA_HRANICA) == false)
                    return DOLNA_HRANICA;
            }
            else if(horna_or_dolna_hranica == DOLNA_HRANICA && is_border(map, r, c, PRAVA_HRANICA) == false)
                return DOLNA_HRANICA;
        }
        else if(0 < c && c < map->cols - 1 && horna_or_dolna_hranica == HORNA_HRANICA && is_border(map, r, c, HORNA_HRANICA) == false)
            return PRAVA_HRANICA;
    }
    else if(r == map->rows - 1)
    {
        if(c == 0)
        {
            if(horna_or_dolna_hranica == DOLNA_HRANICA)
            {
                if(is_border(map, r, c, LAVA_HRANICA) == true && is_border(map, r, c, DOLNA_HRANICA) == false)
                    return LAVA_HRANICA;
                else if(is_border(map, r, c, PRAVA_HRANICA) == true && is_border(map, r, c, LAVA_HRANICA) == false)
                    return PRAVA_HRANICA;
                else if(is_border(map, r, c, PRAVA_HRANICA) == false && is_border(map, r, c, LAVA_HRANICA) == false)
                    return HORNA_HRANICA;
            }
            else if(horna_or_dolna_hranica == HORNA_HRANICA && is_border(map, r, c, LAVA_HRANICA) == false)
                return HORNA_HRANICA;
        }
        else if(c == map->cols - 1)
        {
            if(horna_or_dolna_hranica == DOLNA_HRANICA)
            {
                if(is_border(map, r, c, DOLNA_HRANICA) == false)
                    return LAVA_HRANICA;
                else if(is_border(map, r, c, DOLNA_HRANICA) == true && is_border(map, r, c, PRAVA_HRANICA) == false)
                    return DOLNA_HRANICA;
            }
            else if(horna_or_dolna_hranica == HORNA_HRANICA && is_border(map, r, c, LAVA_HRANICA) == true && is_border(map, r, c, PRAVA_HRANICA) == false)
                return LAVA_HRANICA;
             else if(horna_or_dolna_hranica == HORNA_HRANICA && is_border(map, r, c, LAVA_HRANICA) == false && is_border(map, r, c, PRAVA_HRANICA) == false)
                return DOLNA_HRANICA;
        }
        else if(c != 0 && c != map->cols - 1 && horna_or_dolna_hranica == DOLNA_HRANICA && is_border(map, r, c, DOLNA_HRANICA) == false)
            return LAVA_HRANICA;
    }
    else if(c == 0 && 0 < r && r < map->rows - 1)
    {
        if(horna_or_dolna_hranica == HORNA_HRANICA)
        {
            if(is_border(map, r, c, LAVA_HRANICA) == false)
                return HORNA_HRANICA;
        }
        else
        {
            if(is_border(map, r, c, PRAVA_HRANICA) == true && is_border(map, r, c, LAVA_HRANICA) == false)
                return PRAVA_HRANICA;
            else if(is_border(map, r, c, PRAVA_HRANICA) == false && is_border(map, r, c, LAVA_HRANICA) == false)
                return HORNA_HRANICA;
        }
    }
    else if(c == map->cols -1 && 0 < r && r < map->rows - 1)
    {
        if(horna_or_dolna_hranica == DOLNA_HRANICA)
        {
            if(is_border(map, r, c, PRAVA_HRANICA) == false)
                return DOLNA_HRANICA;
        }
        else
        {
            if(is_border(map, r, c, LAVA_HRANICA) == true && is_border(map, r, c, PRAVA_HRANICA) == false)
                return LAVA_HRANICA;
            if(is_border(map, r, c, LAVA_HRANICA) == false && is_border(map, r, c, PRAVA_HRANICA) == false)
                return DOLNA_HRANICA;
        }
    }
    return -1;
}

/**
 * @brief Zisti, kde sa nachadza pociatocna stena
 * 
 * @param map ukazatel na mapu bludiska v strukture Map
 * @param r riadok, v ktorom sa nachadza policko
 * @param c stlpec, v ktorom sa nachadza policko
 * @param leftright pravidlo pravej/lavej ruky
 * 
 * @return Vracia pociatocnu stenu po ktorej sa ma algoritmus vydat
*/
int start_border(Map *map, int r, int c, int leftright)
{
    int border;
    if(leftright == RIGHT_PATH)
    {
        border = start_border_right(map, r, c);
    }
    else if(leftright == LEFT_PATH)
    {
        border = start_border_left(map, r, c);
    }
    return border;
}

/**
 * @brief Zisti, akym smerom sa ma vydat algoritmus na zaklade start_border
 * 
 * @param map ukazatel na mapu bludiska v strukture Map
 * @param r riadok, v ktorom sa nachadza policko
 * @param c stlpec, v ktorom sa nachadza policko
 * 
 * @return Vracia smer, akym sa ma vydat algoritmus
*/
int start_direction(Map *map, int r, int c, int leftright)
{
    if(leftright == RIGHT_PATH)
    {   
        int border = start_border(map, r, c, RIGHT_PATH);
        switch (border)
        {
            case DOLNA_HRANICA:
                return DOPRAVA;
            case HORNA_HRANICA:
                return DOLAVA;
            case PRAVA_HRANICA:
                return HORE;
            case LAVA_HRANICA:
                return DOLE;
            default:
                return -1;
        }
    }
    else if(leftright == LEFT_PATH)
    {
        int border = start_border(map, r, c, LEFT_PATH);
        switch (border)
        {
            case HORNA_HRANICA:
                return DOPRAVA;
            case DOLNA_HRANICA:
                return DOLAVA;
            case LAVA_HRANICA:
                return HORE;
            case PRAVA_HRANICA:
                return DOLE;
            default:
                return -1;
        }
    }
    return -1;
}

/**
 * @brief Algrotimy na riesenie bludiska podla pravej/lavej ruky
 * 
 * Oba algoritmy sa pozeraju na smer, odkial prisli a na otocenie policka
 * Rukou sa vzdy drzi najblizsej steny a prinutuje opisanu cestu von, pricom sa pozera aj na smer odkial sa prislo
 * 
 * @param map ukazatel na strukturu mapy
 * @param r rad, v ktorom sa momentalne nachadzame
 * @param c stlpec, v ktorom sa momentalne nachadzame
*/
void rpath_algoritmus(Map *map, int r, int c)
{
    int aktualny_smer = start_direction(map, r, c, RIGHT_PATH); 
    if(aktualny_smer == -1)
    {
        fprintf(stdout, "Invalid Entrance\n");
        return;
    }

    while(r > -1 && c > -1 && r < map->rows && c < map->cols)
    {
        printf("%d,%d\n", r + 1, c + 1);
        int horna_or_dolna_hranica = horna_dolna_hranica(r, c);
        if(aktualny_smer == DOPRAVA)
        {
            if(horna_or_dolna_hranica == HORNA_HRANICA)
            {
                if(is_border(map, r, c, PRAVA_HRANICA) == false)
                {
                    aktualny_smer = DOPRAVA;
                    c++;
                }
                else if(is_border(map, r, c, PRAVA_HRANICA) == true && is_border(map, r, c, HORNA_HRANICA) == false)
                {
                    aktualny_smer = HORE;
                    r--;
                }
                else if(is_border(map, r, c, PRAVA_HRANICA) == true && is_border(map, r, c, HORNA_HRANICA) == true)
                {
                    aktualny_smer = DOLAVA;
                    c--;
                }
            }
            else if(horna_or_dolna_hranica == DOLNA_HRANICA)
            {
                if(is_border(map, r, c, DOLNA_HRANICA) == false)
                {
                    aktualny_smer = DOLE;
                    r++;
                }
                else if(is_border(map, r, c, DOLNA_HRANICA) == true && is_border(map, r, c, PRAVA_HRANICA) == false)
                {
                    aktualny_smer = DOPRAVA;
                    c++;
                }
                else if(is_border(map, r, c, DOLNA_HRANICA) == true && is_border(map, r, c, PRAVA_HRANICA) == true)
                {
                    aktualny_smer = DOLAVA;
                    c--;
                }
            }
        }
        else if(aktualny_smer == HORE)
        {
            if(horna_or_dolna_hranica == DOLNA_HRANICA)
            {
                if(is_border(map, r, c, PRAVA_HRANICA) == false)
                {
                    aktualny_smer = DOPRAVA;
                    c++;
                }
                else if(is_border(map, r, c, PRAVA_HRANICA) == true && is_border(map, r, c, LAVA_HRANICA) == false)
                {
                    aktualny_smer = DOLAVA;
                    c--;
                }
                else if(is_border(map, r, c, PRAVA_HRANICA) == true && is_border(map, r, c, LAVA_HRANICA) == true)
                {
                    aktualny_smer = DOLE;
                    r++;
                }
            }
            else if(horna_or_dolna_hranica == HORNA_HRANICA)
            {
                if(is_border(map, r, c, PRAVA_HRANICA) == false)
                {
                    aktualny_smer = DOPRAVA;
                    c++;
                }
                else if(is_border(map, r, c, PRAVA_HRANICA) == true && is_border(map, r, c, HORNA_HRANICA) == false)
                {
                    aktualny_smer = HORE;
                    r--;
                }
                else if(is_border(map, r, c, PRAVA_HRANICA) == true && is_border(map, r, c, HORNA_HRANICA) == true)
                {
                    aktualny_smer = DOLAVA;
                    c--;
                }
            }
        }
        else if(aktualny_smer == DOLE)
        {
            if(horna_or_dolna_hranica == HORNA_HRANICA)
            {
                if(is_border(map, r, c, LAVA_HRANICA) == false)
                {
                    aktualny_smer = DOLAVA;
                    c--;
                }
                else if(is_border(map, r, c, LAVA_HRANICA) == true && is_border(map, r, c, PRAVA_HRANICA) == false)
                {
                    aktualny_smer = DOPRAVA;
                    c++;
                }
                else if(is_border(map, r, c, LAVA_HRANICA) == true && is_border(map, r, c, PRAVA_HRANICA) == true)
                {
                    aktualny_smer = HORE;
                    r--;
                }
            }
            else if(horna_or_dolna_hranica == DOLNA_HRANICA)
            {
                if(is_border(map, r, c, LAVA_HRANICA) == false)
                {
                    aktualny_smer = DOPRAVA;
                    c--;
                }
                else if(is_border(map, r, c, LAVA_HRANICA) == true && is_border(map, r, c, DOLNA_HRANICA) == false)
                {
                    aktualny_smer = DOLE;
                    r++;
                }
                else if(is_border(map, r, c, LAVA_HRANICA) == true && is_border(map, r, c, DOLNA_HRANICA) == true)
                {
                    aktualny_smer = DOLAVA;
                    c++;
                }
            }
        }
        else if(aktualny_smer == DOLAVA)
        {
            if(horna_or_dolna_hranica == HORNA_HRANICA)
            {
                if(is_border(map, r, c, HORNA_HRANICA) == false)
                {
                    aktualny_smer = HORE;
                    r--;
                }
                else if(is_border(map, r, c, HORNA_HRANICA) == true && is_border(map, r, c, LAVA_HRANICA) == false)
                {
                    aktualny_smer = DOLAVA;
                    c--;
                }
                else if(is_border(map, r, c, HORNA_HRANICA) == true && is_border(map, r, c, LAVA_HRANICA) == true)
                {
                    aktualny_smer = DOPRAVA;
                    c++;
                }
            }
            else if(horna_or_dolna_hranica == DOLNA_HRANICA)
            {
               if(is_border(map, r, c, LAVA_HRANICA) == false)
                {
                    aktualny_smer = DOLAVA;
                    c--;
                }
                else if(is_border(map, r, c, LAVA_HRANICA) == true && is_border(map, r, c, DOLNA_HRANICA) == false)
                {
                    aktualny_smer = DOLE;
                    r++;
                }
                else if(is_border(map, r, c, LAVA_HRANICA) == true && is_border(map, r, c, DOLNA_HRANICA) == true)
                {
                    aktualny_smer = DOPRAVA;
                    c++;
                } 
            }
        }
    }
}

void lpath_algoritmus(Map *map, int r, int c)
{
    int aktualny_smer = start_direction(map, r, c, LEFT_PATH);
    if(aktualny_smer == -1)
    {
        fprintf(stdout, "Invalid Entrance\n");
        return;
    }
    
    while(r > -1 && c > -1 && r < map->rows && c < map->cols)
    {
        printf("%d,%d\n", r + 1, c + 1);
        int horna_or_dolna_hranica = horna_dolna_hranica(r, c);
        if(aktualny_smer == DOPRAVA)
        {
            if(horna_or_dolna_hranica == HORNA_HRANICA)
            {
                if(is_border(map, r, c, HORNA_HRANICA) == false)
                {
                    aktualny_smer = HORE;
                    r--;
                }
                else if(is_border(map, r, c, HORNA_HRANICA) == true && is_border(map, r, c, PRAVA_HRANICA) == false)
                {
                    aktualny_smer = DOPRAVA;
                    c++;
                }
                else if(is_border(map, r, c, HORNA_HRANICA) == true && is_border(map, r, c, PRAVA_HRANICA) == true)
                {
                    aktualny_smer = DOLAVA;
                    c--;
                }
            }
            else if(horna_or_dolna_hranica == DOLNA_HRANICA)
            {
                if(is_border(map, r, c, PRAVA_HRANICA) == false)
                {
                    aktualny_smer = DOPRAVA;
                    c++;
                }
                else if(is_border(map, r, c, PRAVA_HRANICA) == true && is_border(map, r, c, DOLNA_HRANICA) == false)
                {
                    aktualny_smer = DOLE;
                    r++;
                }
                else if(is_border(map, r, c, PRAVA_HRANICA) == true && is_border(map, r, c, DOLNA_HRANICA) == true)
                {
                    aktualny_smer = DOLAVA;
                    c--;
                }
            }
        }
        else if(aktualny_smer == HORE)
        {
            if(horna_or_dolna_hranica == DOLNA_HRANICA)
            {
                if(is_border(map, r, c, LAVA_HRANICA) == false)
                {
                    aktualny_smer = DOLAVA;
                    c--;
                }
                else if(is_border(map, r, c, LAVA_HRANICA) == true && is_border(map, r, c, PRAVA_HRANICA) == false)
                {
                    aktualny_smer = DOPRAVA;
                    c++;
                }
                else if(is_border(map, r, c, LAVA_HRANICA) == true && is_border(map, r, c, PRAVA_HRANICA) == true)
                {
                    aktualny_smer = DOLE;
                    r++;
                }
            }
            else if(horna_or_dolna_hranica == HORNA_HRANICA)
            {
                if(is_border(map, r, c, LAVA_HRANICA) == false)
                {
                    aktualny_smer = DOLAVA;
                    c--;
                }
                else if(is_border(map, r, c, LAVA_HRANICA) == true && is_border(map, r, c, HORNA_HRANICA) == false)
                {
                    aktualny_smer = HORE;
                    r--;
                }
                else if(is_border(map, r, c, LAVA_HRANICA) == true && is_border(map, r, c, HORNA_HRANICA) == true)
                {
                    aktualny_smer = DOPRAVA;
                    c++;
                }
            }
        }
        else if(aktualny_smer == DOLE)
        {
            if(horna_or_dolna_hranica == HORNA_HRANICA)
            {
                if(is_border(map, r, c, PRAVA_HRANICA) == false)
                {
                    aktualny_smer = DOPRAVA;
                    c++;
                }
                else if(is_border(map, r, c, PRAVA_HRANICA) == true && is_border(map, r, c, LAVA_HRANICA) == false)
                {
                    aktualny_smer = DOLAVA;
                    c--;
                }
                else if(is_border(map, r, c, PRAVA_HRANICA) == true && is_border(map, r, c, LAVA_HRANICA) == true)
                {
                    aktualny_smer = HORE;
                    r--;
                }
            }
            else if(horna_or_dolna_hranica == DOLNA_HRANICA)
            {
                if(is_border(map, r, c, PRAVA_HRANICA) == false)
                {
                    aktualny_smer = DOPRAVA;
                    c++;
                }
                else if(is_border(map, r, c, PRAVA_HRANICA) == true && is_border(map, r, c, DOLNA_HRANICA) == false)
                {
                    aktualny_smer = DOLE;
                    r++;
                }
                else if(is_border(map, r, c, PRAVA_HRANICA) == true && is_border(map, r, c, DOLNA_HRANICA) == true)
                {
                    aktualny_smer = DOLAVA;
                    c--;
                }
            }
        }
        else if(aktualny_smer == DOLAVA)
        {
            if(horna_or_dolna_hranica == HORNA_HRANICA)
            {
                if(is_border(map, r, c, LAVA_HRANICA) == false)
                {
                    aktualny_smer = DOLAVA;
                    c--;
                }
                else if(is_border(map, r, c, LAVA_HRANICA) == true && is_border(map, r, c, HORNA_HRANICA) == false)
                {
                    aktualny_smer = HORE;
                    r--;
                }
                else if(is_border(map, r, c, LAVA_HRANICA) == true && is_border(map, r, c, HORNA_HRANICA) == true)
                {
                    aktualny_smer = DOPRAVA;
                    c++;
                }
            }
            else if(horna_or_dolna_hranica == DOLNA_HRANICA)
            {
                if(is_border(map, r, c, DOLNA_HRANICA) == false)
                {
                    aktualny_smer = DOLE;
                    r++;
                }
                else if(is_border(map, r, c, DOLNA_HRANICA) == true && is_border(map, r, c, LAVA_HRANICA) == false)
                {
                    aktualny_smer = DOLAVA;
                    c--;
                }
                else if(is_border(map, r, c, DOLNA_HRANICA) == true && is_border(map, r, c, LAVA_HRANICA) == true)
                {
                    aktualny_smer = DOPRAVA;
                    c++;
                }
            }
        }
    }
}
    
int main(int argc, char *argv[])
{
    if(argc < 2)
    {
        fprintf(stderr, "Error, too few arguments. Use --help for further information\n");
        return 1;
    }
    else if(argc > 5)
    {
        fprintf(stderr, "Error, too many arguments. Use --help for further information\n");
        return 1;
    }

    if(strcmp(argv[1], "--help") == 0)
    {
        printf("%s", HELP);
        return 0;
    }
    else if(strcmp(argv[1], "--test") == 0)
    {
        char *subor_s_bludiskom = argv[2];
        Map *mapa = get_map(subor_s_bludiskom);
        if(mapa == NULL)
        {
            fprintf(stdout, "Invalid\n");
            return 1;
        }
        else
        {
            printf("Valid\n");
            free_map(mapa);
            return 0;
        }
    }
    else if(strcmp(argv[1], "--rpath") == 0)
    {
        int vstup_row = atoi(argv[2]) - 1;
        int vstup_col = atoi(argv[3]) - 1;
        char *subor_s_bludiskom = argv[4];
        Map *mapa = get_map(subor_s_bludiskom);

        if(mapa != NULL)
        {
            rpath_algoritmus(mapa, vstup_row, vstup_col);
            free_map(mapa);
        }
        else
        {
            fprintf(stderr, "Invalid\n");
            return -1;
        }
    }
    else if(strcmp(argv[1], "--lpath") == 0)
    {
        int vstup_row = atoi(argv[2]) - 1;
        int vstup_col = atoi(argv[3]) - 1;
        char *subor_s_bludiskom = argv[4];
        Map *mapa = get_map(subor_s_bludiskom);

        if(mapa != NULL)
        {
            lpath_algoritmus(mapa, vstup_row, vstup_col);
            free_map(mapa);
        }
        else
        {
            fprintf(stderr, "Invalid\n");
            return -1;
        }
    }
    else
    {
        fprintf(stderr, "Error, invalid command: use --help for further information.\n");
        return -1;
    }
    return 0;
}