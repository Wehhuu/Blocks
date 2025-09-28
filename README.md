# İçerik Tablosu

1. [Kurulum](#1---kurulum)
    * [Linux Dağıtımlarında Kurulum](#linux-dağıtımlarında-kurulum)
    * [Android Üstünde Termux ile Kurulum](#android-üstünde-termux-ile-kurulum)

2. [Kaldırma](#2---kaldırma)

3. [Modifikayson ve İleri Okuma](#modifikasyon-ve-i̇leri-okuma)
    * [Şekil Tanımlama](#şekil-tanımlama)
    * 🚧 Bu kısmın devamı gelecektir. 🚧


#### ⚠️ Not: Bu proje anlık olarak geliştirme halindedir. Bu dosyaya yeni şeyler eklenebilir veya olan içerikler değiştilebilir. Projenin kendisinde de hala eksik olan bazı özellikler ileride getirilecektir.
# 1 - Kurulum

Eğer projeyi kurmak istiyorsanız aşağıdaki kod ile depoyu klonlayabilirsiniz.

``` bash 
git clone https://github.com/Wehhuu/Blocks
```
Depoyu klonladıktan sonra o dizine gidebilirsiniz.
``` bash 
cd Blocks
```
## Linux Dağıtımlarında Kurulum

Dizine gittikten sonra içerideki Makefile'ı kullanarak istediğiniz şekilde indirme yapabilirsiniz.
``` bash 
make install
```
Bu komut ile programı derleyip `/bin` konumuna taşımış olursunuz.
``` bash 
make build
```
Eğer programı `/bin` konumuna değil de şu anki dizine koymak isterseniz bu komutu çalıştırabilirsiniz.

## Android Üstünde Termux ile Kurulum
Dizine gittikten sonra projeyi derleyebilirsiniz.
``` bash 
make build
```
**⚠️ Not: Termux kullanıyorsanız `make install` komutunu kullanamazsınız. Bunun yerine `make build` ile bulunduğunuz dizinde derleme yapabilirsiniz.**

# 2 - Kaldırma
Platform farketmeksizin:
``` bash
make uninstall 
```
kodunun kullanarak projeyi hem `/bin` adresinden hem de bulunduğunuz konumdan silebilirsiniz.

# Modifikasyon ve İleri Okuma
## Şekil tanımlama:
`(bool layouts[TOTAL_LAYOUTS][LAYOUT_SIZE_Y][LAYOUT_SIZE_X])`

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

