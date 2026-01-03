# Blocks

**Bu depo, tamamiyle C dili kullanarak yazılmış, Tetris benzeri, terminal tabanlı bir oyundur. Oyun, Android (Termux ile) ve Linux dağıtımlarında çalıştırılabilir.**

**Aşağıda, kurulum ve diğer işlemler için bir kılavuz bulacaksınız.**

#### ⚠️ Not: Bu proje aktif olarak geliştirme halindedir. Bu dosyaya yeni şeyler eklenebilir veya olan içerikler değiştilebilir. Projenin kendisinde de hala eksik olan bazı özellikler ileride getirilecektir. 

**⚠️⚠️ÖNEMLİ NOT: Proje bitmediğinden dolayı bazı durumlarda program beklenmedik davranış gösterebilir. Depoyu kullanıyorsanız, bu riski göze almış sayılırsınız.**

#### İlerleme:
- ✅ Blok hareketi
- ✅ Yerçekimi
- ✅ Girdi sistemi
- ✅ Görüntü işleme
- ✅ Makefile (Makefile eklemesi yapıldı)
- ✅ Satır temizleme (Bir takım eksiklikler halen mevcut, ileride düzeltilecek)
- ✅ Rastgelelik eklendi (Zaman tabanlı)
- ✅ Blok hareketleri daha yumuşak hale getirildi ve girdi sistemi daha sağlam yapıldı.
- 🚧 Blokların ve oyun alanının yapıtaşlarına istediğiniz karakteri atama. 
- 🚧 Skor sistemi ve harici kayıt dosyası entegrasyonu
- 🚧 Ana ekran (?)
- 🚧 Kaybetme ekranı (?)
- 🚧 Ana menü (?)

# İçerik Tablosu

1. [Kurulum / Kaldırma](#kurulum--kaldırma)
    * [Kurulum](#kurulum)
        * [Linux Dağıtımlarında Kurulum](#linux-dağıtımlarında-kurulum)
        * [Android Üstünde Termux ile Kurulum](#android-üstünde-termux-ile-kurulum)
    * [Kaldırma](#kaldırma)

2. [Nasıl Oynanır?](#nasıl-oynanır)

3. [Modifikasyon ve İleri Okuma](#modifikasyon-ve-i̇leri-okuma)
    * [Derleme](#derleme)
    * [Kontrol Değiştirme](#kontrol-değiştirme)
    * [Şekil Tanımlama](#şekil-tanımlama)
    * 🚧 Bu kısmın devamı gelecektir. 🚧

4. [Lisans](#lisans)


# Kurulum / Kaldırma

## Kurulum 
Projeyi kurmak için `git`, `make` ve `gcc` programları varsayılan olarak kullanılmaktadır. Eğer `gcc` kullanmak istemiyorsanız ya da `make` kullanmak istemiyorsanız bunu kesinlikle yapabilirsiniz, sadece [Makefile](./Makefile) dosyasında derleme için zorunlu olan flaglare (`LDFLAGS`) bakın ve manuel derlemede bunları kullanın. 

``` bash 
git clone https://github.com/Wehhuu/Blocks
```
Depoyu klonladıktan sonra o dizine gidebilirsiniz.
``` bash 
cd Blocks
```
## Linux Dağıtımlarında Kurulum

Dizine gittikten sonra içerideki Makefile'ı kullanarak istediğiniz şekilde indirme yapabilirsiniz.
Bu komut ile programı derleyip `/bin` konumuna taşımış olursunuz.
``` bash 
make build MODE=release
```
Eğer programı `/bin` konumuna değil de bulunduğunuz dizinde bir `builds` klasörüne koymak isterseniz bu komutu çalıştırabilirsiniz:
``` bash 
make build
```
Kurulumla ilgili detaylı açıklamaları [burada](#derleme) bulabilirsiniz.

## Android Üstünde Termux ile Kurulum
Dizine gittikten sonra projeyi derleyebilirsiniz.
``` bash 
make build
```
**⚠️ Not: Termux kullanıyorsanız programı `make build MODE=release` komutu ile kuramazsınız. Bunun yerine `make build MODE=debug` ile bulunduğunuz dizinde derleme yapabilirsiniz.**


## Kaldırma
Platform farketmeksizin:
``` bash
make clean 
```
kodunun kullanarak projeyi hem `/bin` adresinden hem de bulunduğunuz dizinden silebilirsiniz.

**Not: programı bulunduğunuz dizinden silmek için size sorulduğunda `y` diyin.**


# Nasıl Oynanır?

Kontroller için varsayılan olarak `A` ve `D` tuşları önerilir. Oyundan çıkmak için `Q` harfine basabilirsiniz. (Oyundan çıkarken büyük harf küçük harf varsayılan olarak farkediyor. İsterseniz bunu da değiştirebilirsiniz fakat yanlışlıkla çıkılmayı engellemek adına önerilmez.)

Eğer varsayılan kontrolleri beğenmediyseniz bunları [modifiye edebilirsiniz](#kontrol-değiştirme).


# Modifikasyon ve İleri Okuma
**⚠️ Önemli not: Eğer `main.c` dosyasında herhangi bir değişiklik yaptıysanız projeyi tekrar derlemeniz gerekmektedir. Aşağıda bazı derleme yöntemleri için kılavuzlar bulunmaktadır.** 


## Derleme

**Normal derleme:** Bunun için `make build` seçeneğini kullanabilirsiniz.**

**Debugging uyumlu derleme:** Eğer GDB tarzı bir debugger kullanmak istiyorsanız bunun için `make build MODE=debug` özelliğini kullanabilirsiniz. Böylece bulunduğunuz dizinde debugging destekli bir derleme yapmış olursunuz.**

**Eğer bir debugger kullanmayacaksanız:** `make build MODE=release` ile hem programı optimize edebilirsiniz (varsayılan olarak `gcc` derleyicisinde `-O3` ile derleme yapar) hem de `bin` klasörüne kopyalayabilirsiniz.

## Kontrol Değiştirme
Varsayılan tuş atamalarını kod üstünden değiştirmek için `main.c` dosyasındaki `DIRECTIONS` macrosunu değiştirebilirsiniz. Çıkma tuşunu değiştirmek için de `QUIT_KEY` macrosunu değiştirebilirsiniz.  

## Şekil tanımlama:
Bütün şekiller `bool layouts[TOTAL_LAYOUTS][LAYOUT_SIZE_Y][LAYOUT_SIZE_X]` değişkeni içinde tutulur.

Açıklama:
Her bir şekil iki boyutlu birer `bool`'dur. `layouts` değişkeni üç boyutlu 
bir dizidir.
Tanımlarken bunu göz önünde bulundurun. Aşağıda örnek bir tanımlama 
vardır.

**⚠️ Önemli Not: Eğer yeni bir şekil ekleyecekseniz `TOTAL_LAYOUTS` değişkenini de güncellemeyi sakın unutmayın.** 


### Örnek Kod:
``` C
// //?: Blok tipleri.
bool layouts[TOTAL_LAYOUTS][LAYOUT_SIZE_Y][LAYOUT_SIZE_X] = 
{ 
//* Şeklin üstü | Şeklin altı
{    {1, 1, 1}  ,  {0, 1, 1} },   //* İlk şekil:     ### (1 1 1)
                                  //*                 ## (0 1 1)
                                         
{    {0, 1, 1}  ,  {0, 0, 1} }    //* İkinci şekil:   ## (0 1 1)
                                  //*                  # (0 0 1)

                                  //* ...

// Her yeni şekil için { } koyun, sonra bunun içine şekil 
//boyutlarını karşılayacak şekilde iki boyutlu diziler koyabilirsiniz.
};
```                               
---

# Lisans

Bu proje, [MPL 2.0](./LICENSE) altında lisanslanmıştır.