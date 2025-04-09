Image texture = LoadImage("rendering/pattern.png");
RenderTexture2D vScreen = LoadRenderTexture(320,240);


struct fetchedInst {
    uint8_t palleteBank; 
    uint8_t textureBank;
    int branchAddr;
    uint8_t branchType;
    int jumpAddr;
    uint8_t fetchedData[32];
    uint8_t commandType;
    Vector2 uvData[4];
    Vector2 vertexData[4];
    Color colorData[4];
    bool gouraudEnable;
    float gouraudIntensity;
    
};

struct VDP{
    bool isFirstCommand;
    uint8_t *vramPointer;
    uint32_t startVec;
    uint32_t frameBufferVec;
    uint32_t palleteVectors[4];
    uint32_t textureVectors[4];
    uint32_t currentAddress;
    struct fetchedInst fetchedInst;
    uint8_t computedColour;};

bool testing = true;
struct VDP VDP;
VDP.vramPointer = calloc(640001, 1);
VDP.startVec = 0x09B000;
VDP.frameBufferVec = 0x00;
VDP.palleteVectors[0] = 0x09AF00;
VDP.textureVectors[0] = 0x030000;
FILE *file = fopen("rendering/vramTesting.bin", "rb");
int size = fseek(file, 0, SEEK_END);
size = ftell(file);
fseek(file, 0, SEEK_SET);
fread(VDP.vramPointer, 1, size, file);

struct quad {
    int scanLine;
    bool isTopTri;
    bool isATall;
    bool isDSmall;
    bool isDrawable;
    Vector2 a;
    Vector2 b;
    Vector2 c;
    Vector2 d;
    Vector2 tallestPoint;
    Vector2 midTallPoint;
    Vector2 midLowPoint;
    Vector2 lowestPoint;
    Vector2 tallEdge;
    Vector2 shortEdge;
    int scanStart;
    int scanEnd;
    Image texture;};

struct shading {
    Image texture;
    Color texCol;
    Color shadeCol;
    Color Acol;
    Color Bcol;
    Color Ccol;
    Color Dcol;
    Color computeCols[4];
    uint8_t type; //0 is gradient, 1 is textured(flat shaded), 2 is textured with gouraud, 
    float gouraudIntensity;};


void fetchVertexData(struct VDP *VDP, int VertexNum){
    uint8_t* data = VDP->fetchedInst.fetchedData;
    //vertex
        VDP->fetchedInst.vertexData[VertexNum].x = data[8+(VertexNum*4)+3];
        VDP->fetchedInst.vertexData[VertexNum].y = ((data[8+(VertexNum*4)+1] & 0x01) << 8) | data[8+(VertexNum*4)+2]; //this fucking mess is because of the 9th Y bit
    
    //uv data
        VDP->fetchedInst.uvData[VertexNum].x = data[24+(VertexNum*2)+1];
        VDP->fetchedInst.uvData[VertexNum].y = data[24+(VertexNum*2)];
    //color data
        Color fetchCol;
        fetchCol.r = (data[8+VertexNum*4] & 0b11111000) >> 3;
        fetchCol.g = ((data[8+VertexNum*4] & 0b00000111) << 2) | ((data[8+VertexNum*4+1] & 0b11000000) >> 6);
        fetchCol.b = (data[8+VertexNum*4+1] & 0b00111110) >> 1;
        fetchCol.r = fetchCol.r << 3;
        fetchCol.g = fetchCol.g << 3;
        fetchCol.b = fetchCol.b << 3;
        fetchCol.a = 255;
        VDP->fetchedInst.colorData[VertexNum] = fetchCol;

    if((data[3] & 0b00001000) == 0b00001000) {
        VDP->fetchedInst.gouraudEnable = true;
        VDP->fetchedInst.gouraudIntensity = (data[3] & 0b00000111) / 8.0f;
    }
    else {VDP->fetchedInst.gouraudEnable = false;}
    return;}
float lerp(float a, float b, float t) {
    return (1-t)*a + t*b;}

float getLerpCo(float a, float b, float x){
   return (x-a)/(b-a);}
void DrawPixelMem(int x, int y, Color color, struct VDP *VDP){
    uint32_t address = VDP->frameBufferVec;
    uint8_t r,g,b;
    r = color.r >> 3;
    g = color.g >> 3;
    b = color.b >> 3;
    address += (x + (y * 320)) * 2;
    
    VDP->vramPointer[address] = (r << 2) | (g >>3);
    VDP->vramPointer[address+1] = (g << 5) | b;
    return;}

void DrawFromMem(struct VDP *VDP){
    uint32_t address = VDP->frameBufferVec;
    uint8_t r,g,b;
    for(int i = address; i < 320*240; i++) {
        r = (VDP->vramPointer[i*2] & 0b01111100) << 1;
        g = ((VDP->vramPointer[i*2] & 0b00000011) << 6) | ((VDP->vramPointer[i*2 + 1] & 0b11100000) >> 2);
        b = (VDP->vramPointer[i*2 + 1] & 0b00011111) << 3;
        DrawPixel(i%320,i/320, (Color){r,g,b,255});
    }
    return;}

Color sampleTextureMem(int x, int y, struct VDP *VDP) {
    uint32_t textureAddr = VDP->textureVectors[VDP->fetchedInst.textureBank];
    uint32_t paletteAddr = VDP->palleteVectors[VDP->fetchedInst.palleteBank];
    
    uint8_t colorIndex = VDP->vramPointer[textureAddr+x+(y*256)];
    uint8_t r, g, b;

    r = (VDP->vramPointer[paletteAddr+(colorIndex*2)] & 0b01111100) << 1;
    g = (VDP->vramPointer[paletteAddr+(colorIndex*2)] & 0b00000011) << 6 | VDP->vramPointer[paletteAddr+(colorIndex*2)+1] >> 2;
    b = (VDP->vramPointer[paletteAddr+(colorIndex*2)+1] & 0b00011111) << 3;

    return (Color){r,g,b,255};}
Color mulColor(Color color, float factor) {
    return (Color){color.r*factor, color.g*factor, color.b*factor, 255};}
Color colorLerp(Color a, Color b, float factor) {
    Color ab;
    ab.r = lerp(a.r,b.r, factor);
    ab.g = lerp(a.g,b.g, factor);
    ab.b = lerp(a.b,b.b, factor);
    ab.a = 255;
    return ab;}

int clamp(int val, int min, int max) {
    if (val > max) { val = max;}
    if (val < min) { val = min;}
    return val;}

Color colorMix(struct shading shading, Color a, Color b) {
    Color ab;
    ab.r = clamp(a.r * ((float)b.r/127*shading.gouraudIntensity), 0, 255);
    ab.g = clamp(a.g * ((float)b.g/127*shading.gouraudIntensity), 0, 255);
    ab.b = clamp(a.b * ((float)b.b/127*shading.gouraudIntensity), 0, 255);
    ab.a = 255;
    return ab;}

Color computeGouraud(struct shading shading, Vector2 uv, bool isTextured) {

    shading.computeCols[0] = mulColor(shading.Acol, (1.0f-uv.x)*(1.0f-uv.y));
    shading.computeCols[1] = mulColor(shading.Bcol, uv.x*(1.0f-uv.y));
    shading.computeCols[2] = mulColor(shading.Ccol, uv.y*(1.0f-uv.x));
    shading.computeCols[3] = mulColor(shading.Dcol, uv.x*uv.y);

    Color finalGouraud;
    finalGouraud.r = shading.computeCols[0].r + shading.computeCols[1].r +
        shading.computeCols[2].r + shading.computeCols[3].r;
    finalGouraud.g = shading.computeCols[0].g + shading.computeCols[1].g +
        shading.computeCols[2].g + shading.computeCols[3].g;
    finalGouraud.b = shading.computeCols[0].b + shading.computeCols[1].b +
        shading.computeCols[2].b + shading.computeCols[3].b;
    finalGouraud.a = 255;
    if (isTextured == true) { 
        finalGouraud = colorMix(shading, shading.texCol, finalGouraud);
    }

    return finalGouraud;}

void drawLineMem(int vec1, int vec2, int col1, int col2, struct VDP *VDP) { //all the ints are indexes
    Vector2 start, end;
    start = VDP->fetchedInst.vertexData[vec1];
    end = VDP->fetchedInst.vertexData[vec2];
    Color pixCol, startCol, endCol;
    startCol = VDP->fetchedInst.colorData[col1];
    endCol = VDP->fetchedInst.colorData[col2];
    if(abs(start.x-end.x) > abs(start.y-end.y)) { 
        float lerpY, xCo;
        if(start.x > end.x) {
            Vector2 buffer = start;
            start = end;
            end = buffer;
            Color colBuffer = startCol;
            startCol = endCol;
            endCol = colBuffer; }
        
        for(int x = start.x; x <= end.x; x++){
            xCo = getLerpCo(start.x,end.x,x);
            lerpY = lerp(start.y, end.y, xCo);
            pixCol = colorLerp(startCol, endCol, xCo);
            DrawPixelMem(x, lerpY, pixCol, VDP);
        }
    }
    else{
        float lerpX, yCo;
        if(start.y > end.y) {
            Vector2 buffer = start;
            start = end;
            end = buffer;
            Color colBuffer = startCol;
            startCol = endCol;
            endCol = colBuffer;}

        for(int y = start.y; y <= end.y; y++){
            yCo = getLerpCo(start.y,end.y,y);
            lerpX = lerp(start.x, end.x, yCo);
            pixCol = colorLerp(startCol, endCol, yCo);
            DrawPixelMem(lerpX, y, pixCol, VDP);
        }
    }
    return;}

void textureScanlineTri(struct quad quad, struct shading shading, struct VDP *VDP) {
    float xCo, yCo;
    Color finalCol;
    Vector2 start, offset;
    start = VDP->fetchedInst.uvData[0];
    offset = VDP->fetchedInst.uvData[3];
    

    if (quad.isTopTri == true) {
        //calculate scanlines
        float triCo = getLerpCo(quad.tallestPoint.y, quad.midTallPoint.y, (float)quad.scanLine);
        quad.scanStart = lerp(quad.tallestPoint.x, quad.midTallPoint.x, triCo);
        
        float edgeCo = getLerpCo(quad.tallestPoint.y, quad.tallEdge.y, (float)quad.scanLine);
        quad.scanEnd = lerp(quad.tallestPoint.x, quad.tallEdge.x, edgeCo);
        
        if (quad.scanStart >= quad.scanEnd) { //swap if in wrong order
            int buffer = quad.scanStart;
            quad.scanStart = quad.scanEnd;
            quad.scanEnd = buffer;}
        for( int x = quad.scanStart; x < quad.scanEnd; x ++) {
            if(quad.isATall == true) {
                xCo = getLerpCo(quad.scanStart, quad.b.x, (float)x);
                yCo = getLerpCo(quad.tallestPoint.y, quad.tallEdge.y, (float)quad.scanLine);
                yCo = lerp(yCo, 0.0f, xCo);


                shading.texCol = GetImageColor(shading.texture, start.x + (offset.x * xCo), start.y + (offset.y * yCo));
                if( testing == false) {
                    shading.texCol = sampleTextureMem( start.x+(offset.x * xCo),start.y+(offset.y * yCo), VDP);                     
                }


                if( shading.type == 2) {
                    shading.shadeCol = computeGouraud(shading, (Vector2){xCo,yCo}, true);
                    finalCol = colorLerp(shading.texCol, shading.shadeCol, shading.gouraudIntensity);
                }
                else if (shading.type == 1) {
                    finalCol = shading.texCol;
                }
                else {
                    shading.shadeCol = computeGouraud(shading, (Vector2){xCo,yCo}, false);
                    finalCol = shading.shadeCol;
                }
                DrawPixelMem(x,quad.scanLine, finalCol, VDP);
            }
            else{
                xCo = getLerpCo(quad.a.x, quad.scanEnd, (float)x);
                yCo = getLerpCo(quad.tallestPoint.y, quad.tallEdge.y, (float)quad.scanLine);
                yCo = lerp(0.0f, yCo, xCo);

                shading.texCol = GetImageColor(shading.texture, start.x + (offset.x * xCo), start.y + (offset.y * yCo));
                if( testing == false) {
                    shading.texCol = sampleTextureMem( start.x+(offset.x * xCo),start.y+(offset.y * yCo), VDP);                       
                }

                if( shading.type == 2) {
                    shading.shadeCol = computeGouraud(shading, (Vector2){xCo,yCo}, true);
                    finalCol = colorLerp(shading.texCol, shading.shadeCol, shading.gouraudIntensity);
                }
                else if (shading.type == 1) {
                    finalCol = shading.texCol;
                }
                else {
                    shading.shadeCol = computeGouraud(shading, (Vector2){xCo,yCo}, false);
                    finalCol = shading.shadeCol;
                }
                DrawPixelMem(x,quad.scanLine, finalCol, VDP);

            }
            //DrawPixel(x,quad.scanLine, shading.texCol);
        }
    }
    if (quad.isTopTri == false) {
        float triCo = getLerpCo(quad.lowestPoint.y, quad.midLowPoint.y, (float)quad.scanLine);
        quad.scanStart = lerp(quad.lowestPoint.x, quad.midLowPoint.x, triCo);
        
        float edgeCo = getLerpCo(quad.lowestPoint.y, quad.shortEdge.y, (float)quad.scanLine);
        quad.scanEnd = lerp(quad.lowestPoint.x, quad.shortEdge.x, edgeCo);
        
        if (quad.scanStart >= quad.scanEnd) { //swap if in wrong order
            int buffer = quad.scanStart;
            quad.scanStart = quad.scanEnd;
            quad.scanEnd = buffer;}

        for( int x = quad.scanStart; x < quad.scanEnd; x ++) {
            if(quad.isDSmall == true) {
                xCo = getLerpCo(quad.scanStart, quad.c.x, (float)x);
                yCo = getLerpCo(quad.a.y, quad.d.y, (float)quad.scanLine);
                yCo = lerp(yCo, 1.0f, xCo);

                shading.texCol = GetImageColor(shading.texture, start.x + (offset.x * xCo), start.y + (offset.y * yCo));
                if( testing == false) {
                    shading.texCol = sampleTextureMem( start.x+(offset.x * xCo),start.y+(offset.y * yCo), VDP);                      
                }
                if( shading.type == 2) {
                    shading.shadeCol = computeGouraud(shading, (Vector2){xCo,yCo}, true);
                    finalCol = colorLerp(shading.texCol, shading.shadeCol, shading.gouraudIntensity);
                }
                else if (shading.type == 1) {
                    finalCol = shading.texCol;
                }
                else {
                    shading.shadeCol = computeGouraud(shading, (Vector2){xCo,yCo}, false);
                    finalCol = shading.shadeCol;
                }
                DrawPixelMem(x,quad.scanLine, finalCol, VDP);      
            }
            else{
                DrawPixel(x,(int)quad.scanLine, GREEN);
                xCo = getLerpCo(quad.d.x, quad.scanEnd, (float)x);
                yCo = getLerpCo(quad.b.y, quad.c.y, (float)quad.scanLine);
                yCo = lerp(1.0f, yCo, xCo);

                shading.texCol = GetImageColor(shading.texture, start.x + (offset.x * xCo), start.y + (offset.y * yCo));
                if( testing == false) {
                    shading.texCol = sampleTextureMem( start.x+(offset.x * xCo),start.y+(offset.y * yCo), VDP);                      
                }
                if( shading.type == 2) {
                    shading.shadeCol = computeGouraud(shading, (Vector2){xCo,yCo}, true);
                    finalCol = colorLerp(shading.texCol, shading.shadeCol, shading.gouraudIntensity);
                }
                else if (shading.type == 1) {
                    finalCol = shading.texCol;
                }
                else {
                    shading.shadeCol = computeGouraud(shading, (Vector2){xCo,yCo}, false);
                    finalCol = shading.shadeCol;
                }
                DrawPixelMem(x,quad.scanLine, finalCol, VDP);
            }
            //DrawPixel(x,quad.scanLine, shading.texCol);
        }

        
    }
    return;}

void textureScanLineQuad(struct quad quad, struct shading shading, struct VDP *VDP) {
    Vector2 start, offset;
    start = VDP->fetchedInst.uvData[0];
    offset = VDP->fetchedInst.uvData[3];

    float edge1co = getLerpCo(quad.a.y, quad.d.y, (float)quad.scanLine);
    quad.scanStart = lerp(quad.a.x, quad.d.x, edge1co);

    float edge2co = getLerpCo(quad.b.y, quad.c.y, (float)quad.scanLine);
    quad.scanEnd = lerp(quad.b.x, quad.c.x, edge2co);

    float xCo, yCo, yCo2;
    Color finalCol;
    for(int x = quad.scanStart; x <= quad.scanEnd; x++ ){
        xCo = getLerpCo(quad.scanStart, quad.scanEnd, (float)x);
        yCo = getLerpCo(quad.a.y, quad.d.y, (float)quad.scanLine);
        yCo2 = getLerpCo(quad.b.y, quad.c.y, (float)quad.scanLine);
        yCo = lerp(yCo, yCo2, xCo);

        shading.texCol = GetImageColor(shading.texture, start.x + (offset.x * xCo), start.y + (offset.y * yCo));
        if( testing == false) {
            shading.texCol = sampleTextureMem( start.x+(offset.x * xCo),start.y+(offset.y * yCo), VDP);                    
        }

        if( shading.type == 2) {
            shading.shadeCol = computeGouraud(shading, (Vector2){xCo,yCo}, true);
            finalCol = colorLerp(shading.texCol, shading.shadeCol, shading.gouraudIntensity);
        }
        else if (shading.type == 1) {
            finalCol = shading.texCol;
        }
        else {
            shading.shadeCol = computeGouraud(shading, (Vector2){xCo,yCo}, false);
            finalCol = shading.shadeCol;
        }
        DrawPixelMem(x,quad.scanLine, finalCol, VDP);
    }}

void DrawQuad(struct quad quad, struct shading shading, struct VDP *VDP) {

    if (quad.a.y <= quad.c.y || quad.a.y <= quad.d.y ||
        quad.b.y <= quad.c.y || quad.b.y <= quad.d.y ||
        quad.d.x >= quad.c.x || quad.a.x >= quad.b.x) {
        return;
    }

    if (quad.a.y > quad.b.y) { //sort top vertices
        quad.tallestPoint = quad.a;
        quad.midTallPoint = quad.b;
        quad.tallEdge = quad.d;
        quad.isATall = true;}
    else{
        quad.tallestPoint = quad.b;
        quad.midTallPoint = quad.a;
        quad.tallEdge = quad.c;
        quad.isATall = false;
    }
    if (quad.c.y > quad.d.y) { //sort bottom vertices
        quad.midLowPoint = quad.c;
        quad.lowestPoint = quad.d;
        quad.shortEdge = quad.a;
        quad.isDSmall = true;}
    else {
        quad.midLowPoint = quad.d;
        quad.lowestPoint = quad.c;
        quad.shortEdge = quad.b;
        quad.isDSmall = false;
    }
    //0.0 is bottom!
    for (quad.scanLine = quad.tallestPoint.y; quad.scanLine >= quad.lowestPoint.y; quad.scanLine--) {

        if(quad.scanLine > quad.midTallPoint.y) 
        { //top tri
            quad.isTopTri = true;
            textureScanlineTri(quad, shading, VDP);
        }
        if(quad.scanLine <= quad.midTallPoint.y && quad.scanLine >= quad.midLowPoint.y) 
        { //mid quad
            textureScanLineQuad(quad, shading, VDP);
        }
        if(quad.scanLine < quad.midLowPoint.y) 
        { //bottom tri
            quad.isTopTri = false;
            textureScanlineTri(quad, shading, VDP);
        }
    }}



void fetchCommandData(struct VDP *VDP) {

    if(VDP->isFirstCommand == true) {
        VDP->isFirstCommand = false;
        VDP->currentAddress = VDP->startVec;
    }
    for (int i = 0; i < 32; i++) {
        VDP->fetchedInst.fetchedData[i] = VDP->vramPointer[VDP->currentAddress + i];
    }

    VDP->fetchedInst.commandType = (VDP->fetchedInst.fetchedData[0] & 0b11100000) >> 5;
    VDP->fetchedInst.palleteBank = (VDP->fetchedInst.fetchedData[3] & 0b11000000) >> 6;
    VDP->fetchedInst.textureBank = (VDP->fetchedInst.fetchedData[3] & 0b00110000) >> 4;
    fetchVertexData(VDP, 0);
    fetchVertexData(VDP, 1);
    fetchVertexData(VDP, 2);
    fetchVertexData(VDP, 3);

    VDP->fetchedInst.branchType = (VDP->fetchedInst.fetchedData[0] & 0b00011100) >> 2;
    VDP->fetchedInst.branchAddr = (VDP->fetchedInst.fetchedData[1] << 8) | VDP->fetchedInst.fetchedData[2];
    if (VDP->fetchedInst.fetchedData[0] && 0b00000001) {
        VDP->fetchedInst.branchAddr *= -1;}
    VDP->fetchedInst.jumpAddr = (VDP->fetchedInst.fetchedData[4] << 16) | (VDP->fetchedInst.fetchedData[5] << 8) | VDP->fetchedInst.fetchedData[6];

    switch (VDP->fetchedInst.branchType) {
        case 0x00:
            VDP->currentAddress = VDP->fetchedInst.jumpAddr;
         break;
        case 0x01:
            VDP->currentAddress += VDP->fetchedInst.branchAddr;
         break;
    }
}

void execCommand(struct VDP *VDP) {
    struct quad quad;
    struct shading shading;
    shading.texture = texture;

    quad.a = VDP->fetchedInst.vertexData[0];
    quad.b = VDP->fetchedInst.vertexData[1];
    quad.c = VDP->fetchedInst.vertexData[2];
    quad.d = VDP->fetchedInst.vertexData[3];


    shading.Acol = VDP->fetchedInst.colorData[0];
    shading.Bcol = VDP->fetchedInst.colorData[1];
    shading.Ccol = VDP->fetchedInst.colorData[2];
    shading.Dcol = VDP->fetchedInst.colorData[3];
    switch (VDP->fetchedInst.commandType) {

        case 0x00: //quad mode
            if( VDP->fetchedInst.gouraudEnable == true) {
                shading.type = 2;
                shading.gouraudIntensity = VDP->fetchedInst.gouraudIntensity;
            }
            else {
                shading.type = 1;
            }
            DrawQuad(quad, shading, VDP);
         break;
        case 0x01: //color quad mode
            shading.type = 0;
            DrawQuad(quad, shading, VDP);
         break;
        case 0x02: //sprite
         break;
        case 0x03: //color math
         break;
        case 0x04: //line mode
            drawLineMem(0,1, 0,1, VDP);
         break;
        case 0x05: //polyline mode
            drawLineMem(0,1, 0,1, VDP);
            drawLineMem(1,2, 1,2, VDP);
            drawLineMem(2,3, 2,3, VDP);
            drawLineMem(3,0, 3,0, VDP);
         break;
        case 0x06: //commad mode
         break;
    
    }}
void drawScreen(struct VDP *VDP) {

    BeginTextureMode(vScreen);
    ClearBackground(BLACK);
    fetchCommandData(VDP);

    execCommand(VDP);
    DrawFromMem(VDP);



    EndTextureMode();
    return;
}


