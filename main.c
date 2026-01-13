// Tamamen CLI tabanlı oyun.
#include <ctype.h>
#include <fcntl.h>   // Non-blocking input için ayar yapacağım.
#include <pthread.h> // Girdiyi başka çekirdekte alacağım. Böylece ana thread bloklanmadan girdi alabileceğim.
#include <stdbool.h> // Oyun döngüsünü kontrol edecek temel değişkenler için kullanılacak.
#include <stdio.h>   // Rendering ve input için zorunlu.
#include <stdlib.h>
#include <termios.h> // Terminali raw moda alacağım.
#include <time.h>    // Sırf rastgelelik için şimdilik.
#include <unistd.h>  // Delta kare zamanı için beklerken usleep kullanacağım.

//! ünlem ile işaretlenmiş prototip veya metodlar motorun çalışması için kritik öneme sahiptir.
#pragma region Engine Prototypes
void process_start(void);
void process_frame(void);
void render(void);
void render_ui_content(void);
void render_debug(void);
void* take_input(void* ptr); //!
void set_terminal(void);     //!
void reset_terminal(void);   //!
#pragma endregion Engine Prototypes

// Oyunun çalışmasını sağlayan temel döngüleri kontrol eden ve yapılandıran değişkenler.
// Bunları dikkatli değiştirin, oyunun girdi ve devamı gibi temel sistemlerin çalışmasında
// Kritik rolleri var.
#pragma region Engine Variables
//?: Oyunun ve girdi alma işlemlerinin yürütüldüğü while döngüsünü kontrol eder.
bool should_update;
//?: Girdi, farklı bir threadde asenkron alınıp ana threadde kullanılır.
int input = 0;
//? Terminalin orijinal ayarlarının kayıtlı olduğu değişken.
struct termios original;
#pragma endregion Engine Variables

//? Delta kare ve ekran temizleme motoru da ilgilendirdiği için bu bölüme alındı.
#pragma region Engine Definitions
#define CLEAR_SCREEN printf("\e[1;1H\e[2J") // Ekranı temizler.
#pragma endregion Engine Definitons

//? Bu makrolar ihtiyaca göre düzenlenebilir, yeniden adlandırılabilir.
#pragma region User Definitions
#pragma region Macros
#define DIRECTIONS "AD" // Sağ için 1, sol için 0
#define TOTAL_LAYOUTS 4
#define LAYOUT_SIZE_Y 2
#define LAYOUT_SIZE_X 3
#define QUIT_KEY 'Q'   // q yerine Q seçmemin nedeni yanlışlıkla çıkılma ihtimalini azaltmaktır.
#define GRID_SIZE_X 15 // Oyun alanının x eksenindeki boyutu.
#define GRID_SIZE_Y 25 // Oyun alanının y eksenindeki boyutu.
#pragma endregion Macros

#pragma region Variables
const unsigned int DELTA_TIME = 4000000 / 60; // İki ardışık kare arası mikrosaniye. 1 saniyeye bölümü FPS'i verir.

// Format: \e[48;2;{r};{g};{b}m || \e[44;nm  (n {40 - 47} kümesinin bir elemanıdır.]
char* block_skin = "\e[48;2;37;150;190m  \e[0m"; // ██ (44)
char* wall_skin = "\e[48;2;32;100;215m  \e[0m";  // ██ (47)

// render_debug metodu çağırılsın mı?
bool debug = false;

/// @brief blok nesnesinin fizik etkileşimi ve renderı için gerekli struct.
typedef struct block_part
{
    struct block_part* main; //* Bir bloğun merkezi her zaman bloğun en alt sol noktasında olmalıdır.
    struct block_part* next; //* Her bir blok parçası kendinden sonraki bloğa işaret eder.
} block_part;

// Blok parçalarının olduğu ızgara. Bu ızgaranın her bir hanesi, iki adet işaretçi tutacak.
// Eğer işaretçileri boşsa, bu hanede blok veya blok parçası yok demektir. Fakat işaretçileri
// doluysa, orada bir blok parçası var demektir.
block_part grid[GRID_SIZE_Y][GRID_SIZE_X];

// Yeni bir blok için oyunun hazır olup olmadığına belirleyen değişken.
// Yeni bir blok her oluştuğunda, o blok bir zemine oturana kadar o bloğun
// ızgaradaki adresini tutar. O blok zemine oturunca adreslerine NULL atanır.
block_part current = {NULL, NULL};

// Blok tipleri.
bool layouts[TOTAL_LAYOUTS][LAYOUT_SIZE_Y][LAYOUT_SIZE_X] =
    {
        //*  Şeklin üstü | Şeklin altı
        {{1, 1, 1}, {0, 1, 1}}, //* İlk şekil:  ### (1 1 1)
                                //*              ## (0 1 1)
        {{0, 0, 1}, {0, 0, 1}},

        {{0, 0, 1}, {1, 1, 1}},

        {{1, 1, 0}, {1, 1, 0}},

        /*{{0, 1, 0}, {1, 1, 0}},*/
        /*{{0, 1, 1}, {0, 0, 1}},*/

};
unsigned int score = 0;
#pragma endregion Variables

#pragma region User Prototypes
block_part create_block(void);
void apply_gravity(void);
void clear(void);
int process_input(void);
bool check_cell(block_part* cell, int mode);
void change_pos(int dir);
#pragma endregion User Prototypes

#pragma endregion User Definitions

#pragma region Main
/// @brief Ana döngü.
int main(int argc, char** argv)
{
    should_update = true;
    srand(time(NULL)); // Rastgelelik için tek seferlik seed ayarlaması yapar.

    process_start(); //* İlk kare işlenir. Bu karede girdi yoktur.

    pthread_t input_thread;                                //? İnput için thread oluşturur.
    pthread_create(&input_thread, NULL, take_input, NULL); //? İnput threadine işleyeceği void* verilir.

    //? Temel oyun döngüsü.
    while (should_update)
    {
        process_frame(); //* İlk önce girdiye göre verileri güncelleyelim.

        if (debug)
        {
            render_debug(); //? Eğer debug modunda ise debug içeriğini çiz.
        }

        render_ui_content(); //* Ekranın üstüne gelmesini istediğimden ilk önce UI'ı çizdireceğim.

        render(); //* Güncellenmiş verileri kullanarak render işlemini yapalım.

        if (!should_update)
        {
            break; //? Eğer oyundan oyun normal şekilde bittiyse ekranı temizleme.
        }

        usleep(DELTA_TIME); //* Bekle.

        CLEAR_SCREEN; //* Ekranı temizle.

        if (input == QUIT_KEY)
        {
            should_update = false;
            CLEAR_SCREEN;
            printf("Çıkılıyor...\n");
            break;
        }
    }

    pthread_join(input_thread, NULL); //? Threadi bekle.

    printf("\nProgram sona erdi.\n\n");

    return 0;
}
#pragma endregion Main

#pragma region User Functions
/// @brief Yeni bir blok oluşturur.
/// @return Oluşturduğu bloğun merkezindeki blok parçasının bir kopyasını döndürür.
/// Döndürdüğü blok bir işaretçi değildir, merkez bloğun adresini ve merkez bloktan sonraki
/// bloğun adresini tutan toplamda iki adet işaretçiyi barındıran bir değer döndürür.
block_part create_block(void)
{
    int selected_layout = rand() % TOTAL_LAYOUTS;
    bool is_main_assigned = false;

    //?: Bu değişkenin main işaretçisi sabit bir değer alırken, next işaretçisi başka bir
    // kullanmamak için önce kendisine atanıyor, daha sonra next işaretçisine ondan sonraki
    // hücre atanıyor.
    block_part cell;

    int floors[2] = {1, (GRID_SIZE_X - 1) / 2 - LAYOUT_SIZE_X / 2};

    for (int i = LAYOUT_SIZE_Y - 1; i >= 0; i--)
    {
        for (int j = 0; j < LAYOUT_SIZE_X; j++)
        {
            if (grid[floors[0] + i][floors[1] + j].main != NULL)
            {
                should_update = false;
                cell.main = NULL;
                cell.next = NULL;

                return cell;
            }
            else if (!is_main_assigned && layouts[selected_layout][i][j] == true)
            {
                cell.main = &grid[floors[0] + i][floors[1] + j];
                grid[floors[0] + i][floors[1] + j].main = cell.main;
                cell.next = cell.main;
                is_main_assigned = true;
            }
            else if (is_main_assigned && layouts[selected_layout][i][j] == true)
            {
                grid[floors[0] + i][floors[1] + j].main = cell.main;
                cell.next->next = &grid[floors[0] + i][floors[1] + j];
                cell.next = &grid[floors[0] + i][floors[1] + j];
            }
        }
    }

    return *(cell.main);
}

/// @brief Bütün blokları en alttan başlayarak aşağı indirmeye çalışır.
/// @param
void apply_gravity(void)
{
    for (int y = GRID_SIZE_Y - 1; y >= 0; y--)
    {
        for (int x = 0; x < GRID_SIZE_X; x++)
        {
            if (grid[y][x].main != NULL && grid[y][x].main == &grid[y][x]) //? Eğer bir bloğun parçası ise.
            {
                if (check_cell(&grid[y][x], 0))
                {
                    block_part* cell = &grid[y][x];
                    block_part next_holder;
                    next_holder.main = cell->main + GRID_SIZE_X;

                    while (cell->next != NULL)
                    {
                        //* Yeni adrese değer geç.
                        (cell + GRID_SIZE_X)->next = cell->next + GRID_SIZE_X;
                        (cell + GRID_SIZE_X)->main = next_holder.main;

                        //* Sonrayı kaydet.
                        next_holder.next = cell->next;

                        //* Şu anki adresi temizle.
                        cell->next = NULL;
                        cell->main = NULL;

                        //* Yeni adrese geç.
                        cell = next_holder.next;
                    }

                    (cell + GRID_SIZE_X)->next = NULL;
                    (cell + GRID_SIZE_X)->main = next_holder.main;

                    cell->next = NULL;
                    cell->main = NULL;

                    if (&grid[y][x] == current.main)
                    {
                        current.main += GRID_SIZE_X;
                        current.next += GRID_SIZE_X;
                    }
                }
            }
        }
    }
}

/// @brief Bütün hücreleri dolu olan satırları aşağıdan yukarıya doğru temizler.
/// @param
void clear(void)
{
    for (int i = GRID_SIZE_Y; i > 0; i--)
    {
        //?: Satır kontrolünden sonra bu değişkene göre satır temizleme yapacağız.
        bool ready = true;

        for (int j = 0; j < GRID_SIZE_X - 1; j++)
        {
            //?: Bir tane bile boş hane varsa bu satırı atla. Boş yere durmayalım.
            if (grid[i][j].main == NULL || grid[i][j].main == current.main)
            {
                ready = false;
                break;
            }
        }

        if (ready)
        {
            // Bu aşamada sürekli satırlar arasında konum değiştireceğimiz için ne zaman satır sonuna geldiğimizi anlayamayız. Bu nedenle bu değişkeni kullanarak satır sonuna geldiğimizi anlayacağız.
            int x_pos = 0;
            int row = 0;

            while (x_pos < GRID_SIZE_X)
            {
                block_part storage;
                storage.main = NULL;
                storage.next = NULL;

                //* Bloğun merkezi daha aşağıda kalıyorsa.
                if (grid[i][x_pos].main > &grid[i][x_pos])
                {
                    // Merkez adres orijinali ile aynı kaldıkça ve şu anki adres ile başlangıç konumu arasında en az bir satır oldukça merkezden yukarı çıkmaya çalış.
                    for (block_part* diver = grid[i][x_pos].main; diver->next != NULL && diver->main == grid[i][x_pos].main && (diver - &grid[i][x_pos]) >= GRID_SIZE_X; diver = diver->next)
                    {
                        // Üst satırlarda kalan ilk parça.
                        storage.main = diver;
                    }

                    /// Şu anda, hedef satırdan önceki son bloğu biliyoruz. Tek yapmamız gereken o bloğun next değerini sıfırlamak.
                    if (storage.main != NULL)
                    {
                        (storage.main)->next = NULL;
                    }

                    // Şimdi az önceye geri dönüp kalan blokları silebiliriz.
                    for (block_part* diver = &grid[i][x_pos]; diver->main == storage.main && diver != storage.main; diver++) // Eğer şu anki hücre az öncekinin parçasıysa onu silelim.
                    {
                        row++;
                        diver->main = NULL;
                        diver->next = NULL;
                    }
                }

                //* Bloğun merkezi bu satırda. (Not: Her blok için sadece bir kere işlem yapıyoruz.
                //* Ve sağdan sola gittiğimize göre eğer o bloğun merkezi o satırdaysa merkez, o bloktan
                //* erişebileceğimiz ilk parça olmak zorunda. Bu zaten bu tasarım mimarisin ana faydalarından da biri.)
                if (grid[i][x_pos].main == &grid[i][x_pos])
                {
                    // Bütün blok aynı satırda mı?
                    bool same_row = true;

                    // Gidebildiğimiz kadar gidip en sonki bloğa ulaşalım. Bakalım aynı satırda mı?
                    for (block_part* diver = &grid[i][x_pos]; diver != NULL && diver->main == grid[i][x_pos].main; diver = diver->next)
                    {
                        if (labs(diver - &grid[i][x_pos]) < GRID_SIZE_X) // Aynı satırdalarsa farkları satır uzunluğundan küçük olmak zorunda.
                        {
                            same_row = true;
                            storage.main = diver;
                        }
                        else // Bu satırda olmayan ilk bloğu arıyorsam bu şekilde kendimi sağlama almalıyım ki başka bir blok varsa bile ilgilenmeyeyim.
                        {
                            same_row = false;
                            storage.main = diver; // Aradığımız şey bloğun merkez hücrenin olduğu satırdan başka satırda bulunan ilk parçası.
                            break;
                        }
                    }

                    //? Bütün blok aynı satırda.
                    if (same_row)
                    {
                        block_part old_block;
                        block_part tmp;

                        tmp.next = grid[i][x_pos].next;
                        old_block.main = grid[i][x_pos].main;

                        // Tüm bloğu sil.
                        for (block_part* diver = &grid[i][x_pos]; diver != NULL && diver->main == old_block.main; diver = tmp.next, tmp.next = diver->next)
                        {
                            row++;
                            diver->next = NULL;
                            diver->main = NULL;
                        }
                    }

                    //? Bloğun üstü başka satırda.
                    else
                    {
                        // O satırda olmayan ilk bloğu yeni merkez yap ve silinecek satırda olmayan bütün bloklar için merkezi blok değerini de güncelle.
                        for (block_part* diver = storage.main; diver != NULL; diver = diver->next)
                        {
                            diver->main = storage.main;
                        }

                        block_part tmp;
                        tmp.next = grid[i][x_pos].next;

                        // Eski merkezden başlayıp yeni merkeze kadar o bloğun tüm üyelerini temizle.
                        for (block_part* diver = &grid[i][x_pos]; diver != storage.main; diver = tmp.next, tmp.next = diver->next)
                        {
                            row++;
                            diver->next = NULL;
                            diver->main = NULL;
                        }
                    }
                }

                score += row * row;
                x_pos += row;
            }

            //! Kontrol edin.
            block_part* eraser = &grid[i][0];

            while (eraser != &grid[i][GRID_SIZE_X])
            {
                eraser->main = NULL;
                eraser->next = NULL;
                eraser++;
            }
        }
    }
}

/// @brief O karedeki girdiyi işleyerek bir yön döndürür.
/// @return Alınan girdiyi kabul edilen girdi ile kıyaslayarak uygun değeri döndürür. (-1, 0, 1)
int process_input(void)
{
    if (toupper(input) == DIRECTIONS[0])
    {
        return -1;
    }
    else if (toupper(input) == DIRECTIONS[1])
    {
        return 1;
    }
    return 0;
}

/// @brief Bloğun istenen hareket yönüne gidip gidemeyeceğini kontrol eder.
/// @param cell Merkez hücrenin adresi.
/// @param mode İstenen hareket modu. 0 düşey, -1 ve 1 de istenen yatay hareket için girilir.
/// @return Verilen merkez bloğun istenen hareket yönünün mümkün olup olmadığını döndürür.
bool check_cell(block_part* cell, int mode)
{
    if ((&grid[GRID_SIZE_Y - 1][GRID_SIZE_X - 1] - cell - GRID_SIZE_X) < sizeof(block_part))
    {
        if (cell->main == current.main)
        {
            current.main = NULL;
            current.next = NULL;
        }

        return false;
    }

    if (mode == 0)
    {
        for (int i = 0; cell->next != NULL && i < LAYOUT_SIZE_Y * LAYOUT_SIZE_X; cell = cell->next, i++)
        {
            if ((cell + GRID_SIZE_X)->main != NULL && (cell + GRID_SIZE_X)->main != cell->main)
            {
                if (cell->main == current.main)
                {
                    current.main = NULL;
                    current.next = NULL;
                }

                return false;
            }
        }

        return true;
    }

    else if (abs(mode) == 1)
    {
        for (int i = 0; cell->next != NULL && i < LAYOUT_SIZE_Y * LAYOUT_SIZE_X; cell = cell->next, i++)
        {
            //? En sağda veya soldayken daha da gitmesine engel olalım.
            if (
                mode < 0
                    ? ((cell - &grid[0][0]) % GRID_SIZE_X >= 1)
                    : ((&grid[GRID_SIZE_Y - 1][GRID_SIZE_X - 1] - cell) % GRID_SIZE_X > 1)) // abs(((cell - &blocks[0][0]) % GRID_SIZE_X) - GRID_SIZE_X) > 1)//!  || (&blocks[GRID_SIZE_Y][GRID_SIZE_X] - cell ) % GRID_SIZE_X != GRID_SIZE_X - 1 mutlak değerle yapamadığm zamanlardan falan kalma galiba. Bu not öyle iyi bir amaca hizmet ediyor değil yani.
            {
                if ((cell + mode)->main != NULL && (cell + mode)->main != cell->main)
                {
                    return false;
                }
            }
            else
            {
                return false;
            }
        }

        return true;
    }

    if (cell->main == current.main)
    {
        current.main = NULL;
        current.next = NULL;
    }

    return false;
}

/// @brief Verilen yöne göre şu anki bloğun konumunu değiştirir.
/// @param dir  -1 sol 1 sağ olmak üzere iki değer alır.
void change_pos(int dir)
{
    if (dir == 0)
    {
        return;
    }
    else if (!check_cell(current.main, dir))
    {
        return;
    }

    block_part* cell = current.main;
    block_part secondary_storage;
    block_part storage;

    bool success = false;
    bool is_this_last_piece = false;

    current.main += dir;
    current.next += dir;

    for (int i = 0; i <= LAYOUT_SIZE_X * LAYOUT_SIZE_Y; i++)
    {
        if (dir == 1)
        {
            secondary_storage.main = (cell + dir);
            secondary_storage.next = (cell + dir)->next;

            is_this_last_piece =
                (secondary_storage.next == NULL && (cell + dir)->main == current.main - (dir))
                    ? true
                    : false;

            (cell + dir)->next =
                success
                    ? storage.next + dir
                    : cell->next + dir;

            (cell + dir)->main = current.main;

            //? Şu anki hücre geçmiş bir adımın parçası mı?
            if (cell->main != current.main)
            {
                cell->next = NULL;
                cell->main = NULL;
            }

            storage = secondary_storage;

            //? Depo edilen yerde değer var mı?
            if (is_this_last_piece)
            {
                cell = storage.main;

                (cell + dir)->next = NULL;
                (cell + dir)->main = current.main;

                if (cell->main != current.main)
                {
                    cell->next = NULL;
                    cell->main = NULL;
                }

                break;
            }
            else if (storage.next == NULL)
            {
                //* Depolanan adres boşmuş, az önce değerini yazdığımız yerden devam, veri kaybı yok.
                cell = (storage.main)->next - dir; //? dir çıkartmamızın sebebi az önce veri yazarken ileriye yönelik ekleme yapmış olmamızdı. O yüzden şimdi de çıkarıyorum.
                success = false;
            }
            else
            {
                //* Veri depolama işe yaradı, veri kaybı engellendi.
                cell = storage.main;
                success = true;
            }

            //? Eğer veri depolama başarılı ise burası çalışmaz. Çünkü bunun çalışması için depolanan bloğun sondan sonra gelmesi lazım ki cell->next NULL olabilsin.
            if (cell->next == NULL)
            {
                (cell + dir)->next = NULL;
                (cell + dir)->main = current.main;

                if (cell->main != current.main)
                {
                    cell->next = NULL;
                    cell->main = NULL;
                }

                break;
            }
        }

        if (dir == -1)
        {
            storage.next = cell->next;

            (cell + dir)->main = current.main;
            (cell + dir)->next = storage.next + dir;

            cell->next = NULL;
            cell->main = NULL;

            if (storage.next == NULL)
            {
                (cell + dir)->next = NULL;
                (cell + dir)->main = current.main;

                cell->next = NULL;
                cell->main = NULL;
                break;
            }

            cell = storage.next;
        }
    }
}
#pragma endregion User Functions

#pragma region Game Logic
/// @brief Oyunun ilk karesini işle. Bu karede girdi kullanmayın.
void process_start(void)
{
    current = create_block();
}

/// @brief Kareyi işle, inputa bağlı olarak değerleri güncelle, yerçekimini uygula.
void process_frame(void)
{
    if (current.main == NULL)
    {
        //* Burası düzgünce çalışıyor.
        current = create_block();
    }
    else
    {
        int dir = process_input();

        change_pos(dir);

        apply_gravity();

        clear();
    }
}

/// @brief Her bir karede terminal ekranına çizim yapacak metod.
void render(void)
{
    for (int l = 0; l < GRID_SIZE_X + 3; l++)
    {
        printf("%s", wall_skin);
    }

    printf("\n");

    for (int i = 0; i < GRID_SIZE_Y; i++)
    {
        printf("%s ", wall_skin);

        for (int j = 0; j < GRID_SIZE_X; j++)
        {
            printf("%s", grid[i][j].main != NULL ? block_skin : "  ");
        }

        printf(" %s\n", wall_skin);
    }

    for (int l = 0; l < GRID_SIZE_X + 3; l++)
    {
        printf("%s", wall_skin);
    }

    printf("\n");
}

/// @brief Oyuncunun skorunu vs.sini ekrana yazmak için metod.
void render_ui_content(void)
{
    printf("\tPuan: %i", score);

    printf("\n"); //! Ne yazdırırsan yazdır ne kadar yazarsan yaz her zaman işin bitince alt satıra geç ki oyun çizilirken yanlışlık olmasın.
}

/// @brief Hata giderme amaçlı bilgilerin yazılacağı pencere.
void render_debug(void)
{
    printf("-------------Debug Window-------------\n");
    printf("Anlık girdi: %i\nKarşılık gelen tuş: %c\n", input, input);
    printf("--------------------------------------\n");

    printf("\n");
}
#pragma endregion Game Logic

// Bundan sonraki kısım oyunun çalışması için temel fonksiyonları sunar.
// Input ve input için gerekli olan terminal ayarları burada yapılır.
#pragma region Abstractions

/// @brief Kullanıcı girdisini dinler. Farklı bir threadde çalıştığı
/// için ana oyun işlemlerini bozmaz. Input için gerekli terminal ayarlarını
/// bu metod yönetir, bu metod dışında terminal ayarı yapmayın.
void* take_input(void* ptr)
{
    set_terminal();
    int tmp = -1;

    while (should_update)
    {
        tmp = getchar();
        tcflush(STDIN_FILENO, TCIFLUSH); // Her okumadan sonra tty io buffer'ının okunmamış input kısmını tamamen sil. Böylece uzun ANSII kodlu yön tuşları falan basılırsa bile sadece bir kareliğine işlem görsünler, yolu tıkamasınlar.

        input = tmp == -1 ? 0 : tmp;

        usleep(DELTA_TIME);
    }

    return NULL;
}

/// @brief Terminali canonical moddan raw moda alır, non-blocking input ayarlarını yapar.
void set_terminal(void)
{
    struct termios attr;

    atexit(reset_terminal);

    tcgetattr(STDIN_FILENO, &attr);
    original = attr;
    attr.c_lflag &= ~(ECHO | ICANON); // Terminal girdi modunu non-canonical yap./
    tcsetattr(STDIN_FILENO, TCSANOW, &attr);

    fcntl(STDIN_FILENO, F_SETFL, O_NONBLOCK);
}

void reset_terminal(void)
{
    tcsetattr(STDIN_FILENO, TCSAFLUSH, &original);
}

#pragma endregion Abstractions
