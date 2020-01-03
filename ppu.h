#ifndef PPU_H
#define PPU_H
#include <cstdint>


enum PPUMASK_BITS
{
    PPUMASK_GRAYSCALE = 1<<0,
    PPUMASK_SHOW_BACKGROUND_LEFT = 1<<1,
    PPUMASK_SHOW_SPRITE_LEFT = 1<<2,
    PPUMASK_SHOW_BACKGROUND = 1<<3,
    PPUMASK_SHOW_SPRITE = 1<<4,
    PPUMASK_RED = 1<<5,
    PPUMASK_GREEN = 1<<6,
    PPUMASK_BLUE = 1<<7,
};

class PPU
{
    public:
        PPU();
        ~PPU();

        //copia as pattern table pra memoria da ppu
        //se local=0; sobreescreve pattern table 1 (posicao 0x0)
        //se local=1; sobreescreve pattern table 2 (posicao 0x1000)
        //dados deve ser um vetor de 0x1000(4096) bytes
        void NovaPatternTable(uint8_t* dados, int local=0);


        // ------ 0x2000
        void ppucrtl(const uint8_t new_flag);

        // ------ 0x2001
        void ppumask(const uint8_t data);

        // ----- 0x2002
        uint8_t ppustatus();

        //---- 0x2003
        void SetOamAddr(const uint8_t addr);

        // ---- 0x2004
        uint8_t ReadOamData();
        void WriteOamData(const uint8_t byte);

        // ---- 0x2005 (ppuscroll)
        //aqui primeiro escreve o byte da posicao X e depois o byte da posicao y
        //geralmente se escreve nesta porta 2 vezes
        void ppuscroll(const uint8_t byte);


        // ----- 0x2006
        //aqui se escrevem 2 bytes que sao um ponteiro para a memoria interna da ppu
        void ppuaddr(const uint8_t byte);

        // ------ 0x2007
        //aqui eh lido ou escrito da memoria da ppu.
        //o local da operacao eh definido pelos bytes escritos na porta anterior
        void ppudata_write(const uint8_t byte);
        uint8_t ppudata_read();

        // ----- 0x4014
        //manda 256bytes da ram para a OAM
        //recebe um vetor de 256 elementos para mandar pra oam
        void oamdma_write(const uint8_t* c);



         void Draw();

         uint8_t framebuffer[240][256];

         bool NMI_ativa();

         bool VerticalBlank(); //se retornar true, a cpu deve executar uma nmi

         inline void write_oam_byte(int pos, uint8_t byte)
         {
             OAM[pos] = byte;
         }

    protected:
        uint8_t memoria[16384];
        uint8_t OAM[256];//object atribute memory (informação dos sprites)


        uint8_t m_ppuctrl;
        uint8_t m_ppumask;
        uint8_t m_oamaddr;

        uint8_t m_last_bit_written;


        uint8_t proximo_byte_no_ptr; //controla se o proximo byte escrito na porta 0x2006 vai pro primeiro ou segundo byte
        uint16_t ponteiro;

        uint8_t oam_addr; //ponteiro dentro da memoria oam

        uint8_t ultima_coisa_escrita; //TODO implementar essa merda

        bool vertical_blank;
        bool sprite_0_hit;
        bool sprite_overflow;

        uint8_t proximo_scroll;
        uint8_t scroll_x;
        uint8_t scroll_y;

        void drawTile(uint8_t x, uint8_t y, uint8_t paleta, uint8_t pattern_table, uint8_t tile, uint8_t flipx=0, uint8_t flipy=0);

    private:
};

void ppu_register_write(uint16_t addr, uint8_t val);
uint8_t ppu_register_read(uint16_t addr);

#endif // PPU_H
