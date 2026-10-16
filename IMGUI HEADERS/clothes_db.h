// Auto-generated from shopping.dat — store-based clothing database
// Stores: 0=Binco 1=ProLaps 2=SubUrban 3=Zip 4=Victim 5=DidierSachs
// Cats: 0=Hats(16) 1=Torso(0) 2=Legs(2) 3=Shoes(3) 4=Glasses(15) 5=Watches(14) 6=Necklaces(13)
#pragma once

struct ClothesDBItem {
    const char* display;
    const char* tex;
    const char* model;
    int bodyPart;
};

struct ClothesDBCat {
    const ClothesDBItem* items;
    int count;
};

// Binco / Hats (12)
static const ClothesDBItem g_clothes_s0_hats[] = {
    {"Band Black", "bandblack", "bandana", 16},
    {"Band Black 2", "bandblack2", "bandknots", 16},
    {"Band Blue", "bandblue", "bandana", 16},
    {"Band Blue 2", "bandblue2", "bandknots", 16},
    {"Band Gang", "bandgang", "bandana", 16},
    {"Band Gang 2", "bandgang2", "bandknots", 16},
    {"Band Red", "bandred", "bandana", 16},
    {"Band Red 2", "bandred2", "bandknots", 16},
    {"Cap Truck", "captruck", "captruck", 16},
    {"Capknit Grn", "capknitgrn", "capknit", 16},
    {"Cowboy", "cowboy", "cowboy", 16},
    {"Hattiger", "hattiger", "cowboy", 16},
};

// Binco / Torso (12)
static const ClothesDBItem g_clothes_s0_torso[] = {
    {"Field", "field", "field", 0},
    {"Hoody A Green", "hoodyAgreen", "hoodyA", 0},
    {"Shirt B Check", "shirtbcheck", "shirtb", 0},
    {"Shirt Bgang", "shirtbgang", "shirtb", 0},
    {"T-Shirt Blu Nts", "tshirtblunts", "tshirt", 0},
    {"T-Shirt Eris Orn", "tshirterisorn", "tshirt", 0},
    {"T-Shirt Erisyell", "tshirterisyell", "tshirt", 0},
    {"T-Shirt Ilovels", "tshirtilovels", "tshirt", 0},
    {"T-Shirt White", "tshirtwhite", "tshirt", 0},
    {"Tracky Top 2 Eris", "trackytop2eris", "trackytop1", 0},
    {"Vest", "vest", "vest", 0},
    {"Vest Black", "vestblack", "vest", 0},
};

// Binco / Legs (12)
static const ClothesDBItem g_clothes_s0_legs[] = {
    {"Biegetr", "biegetr", "chinosb", 2},
    {"Denims Gang", "denimsgang", "jeans", 2},
    {"Jeans Denim", "jeansdenim", "jeans", 2},
    {"Legs Black", "legsblack", "legs", 2},
    {"Legsheart", "legsheart", "legs", 2},
    {"Tracktr", "tracktr", "tracktr", 2},
    {"Tracktr Eris", "tracktreris", "tracktr", 2},
    {"Tracktr Gang", "tracktrgang", "tracktr", 2},
    {"Worktr Camo Grn", "worktrcamogrn", "worktr", 2},
    {"Worktr Camo Gry", "worktrcamogry", "worktr", 2},
    {"Worktr Grey", "worktrgrey", "worktr", 2},
    {"Worktr Khaki", "worktrkhaki", "worktr", 2},
};

// Binco / Shoes (9)
static const ClothesDBItem g_clothes_s0_shoes[] = {
    {"Bask 1 Eris", "bask1eris", "bask1", 3},
    {"Bask 2 Semi", "bask2semi", "bask1", 3},
    {"Cowboyboot 2", "cowboyboot2", "biker", 3},
    {"Flipflop", "flipflop", "flipflop", 3},
    {"Sandal", "sandal", "flipflop", 3},
    {"Sandalsock", "sandalsock", "flipflop", 3},
    {"Sneaker Binc Blk", "sneakerbincblk", "sneaker", 3},
    {"Sneaker Binc Blu", "sneakerbincblu", "sneaker", 3},
    {"Sneaker Bincgang", "sneakerbincgang", "sneaker", 3},
};

// Binco / Glasses (7)
static const ClothesDBItem g_clothes_s0_glasses[] = {
    {"Band Black 3", "bandblack3", "bandmask", 15},
    {"Band Blue 3", "bandblue3", "bandmask", 15},
    {"Band Gang 3", "bandgang3", "bandmask", 15},
    {"Band Red 3", "bandred3", "bandmask", 15},
    {"Eyepatch", "eyepatch", "eyepatch", 15},
    {"Groucho", "groucho", "grouchos", 15},
    {"Zorro", "zorro", "zorromask", 15},
};

// Binco / Watches (2)
static const ClothesDBItem g_clothes_s0_watches[] = {
    {"Watch Pink", "watchpink", "watch", 14},
    {"Watch Yellow", "watchyellow", "watch", 14},
};

// Binco / Necklaces (2)
static const ClothesDBItem g_clothes_s0_necklaces[] = {
    {"Dogtag", "dogtag", "neck", 13},
    {"Neck Africa", "neckafrica", "neck", 13},
};

// ProLaps / Hats (10)
static const ClothesDBItem g_clothes_s1_hats[] = {
    {"Bikerhelmet", "bikerhelmet", "bikerhelmet", 16},
    {"Boxingcap", "boxingcap", "boxingcap", 16},
    {"Cap Gang", "capgang", "cap", 16},
    {"Cap Gang Back", "capgangback", "capback", 16},
    {"Cap Gang Over", "capgangover", "capovereye", 16},
    {"Cap Gang Side", "capgangside", "capside", 16},
    {"Cap Gang Up", "capgangup", "caprimup", 16},
    {"Helmet", "helmet", "helmet", 16},
    {"Hockey", "hockey", "hockeymask", 16},
    {"Moto", "moto", "moto", 16},
};

// ProLaps / Torso (12)
static const ClothesDBItem g_clothes_s1_torso[] = {
    {"Bandits", "bandits", "baseball", 0},
    {"Bask Balldrib", "baskballdrib", "baskball", 0},
    {"Bask Ballrim", "baskballrim", "baskball", 0},
    {"Bb Jersey", "bbjersey", "sleevt", 0},
    {"Bballjackr Star", "bballjackrstar", "bbjack", 0},
    {"Bbjackrim", "bbjackrim", "bbjack", 0},
    {"Hockey Top", "hockeytop", "sweat", 0},
    {"Shellsuit", "shellsuit", "trackytop1", 0},
    {"Sixtyniners", "sixtyniners", "tshirt", 0},
    {"T-Shirt Pro Blk", "tshirtproblk", "tshirt", 0},
    {"T-Shirt Pro Red", "tshirtprored", "tshirt", 0},
    {"Tracky Top 1 Pro", "trackytop1pro", "trackytop1", 0},
};

// ProLaps / Legs (7)
static const ClothesDBItem g_clothes_s1_legs[] = {
    {"Bbshort Red", "bbshortred", "boxingshort", 2},
    {"Bbshort Wht", "bbshortwht", "boxingshort", 2},
    {"Boxshort", "boxshort", "boxingshort", 2},
    {"Shellsuit Tr", "shellsuittr", "tracktr", 2},
    {"Tracktr Blue", "tracktrblue", "tracktr", 2},
    {"Tracktr Pro", "tracktrpro", "tracktr", 2},
    {"Tracktr Whstr", "tracktrwhstr", "tracktr", 2},
};

// ProLaps / Shoes (10)
static const ClothesDBItem g_clothes_s1_shoes[] = {
    {"Bask 1 Pro Blk", "bask1problk", "bask1", 3},
    {"Bask 1 Pro Wht", "bask1prowht", "bask1", 3},
    {"Boxingshoe", "boxingshoe", "biker", 3},
    {"Conv Pro Blk", "convproblk", "conv", 3},
    {"Conv Pro Blu", "convproblu", "conv", 3},
    {"Conv Pro Grn", "convprogrn", "conv", 3},
    {"Hitop", "hitop", "bask1", 3},
    {"Sneaker Pro Blu", "sneakerproblu", "sneaker", 3},
    {"Sneaker Pro Red", "sneakerprored", "sneaker", 3},
    {"Sneaker Pro Wht", "sneakerprowht", "sneaker", 3},
};

// ProLaps / Glasses (2)
static const ClothesDBItem g_clothes_s1_glasses[] = {
    {"Glasses 01 Dark", "glasses01dark", "glasses01", 15},
    {"Glasses 04 Dark", "glasses04dark", "glasses04", 15},
};

// ProLaps / Watches (2)
static const ClothesDBItem g_clothes_s1_watches[] = {
    {"Watch Pro", "watchpro", "watch", 14},
    {"Watch Pro 2", "watchpro2", "watch", 14},
};

// ProLaps / Necklaces (2)
static const ClothesDBItem g_clothes_s1_necklaces[] = {
    {"Neck Saints", "necksaints", "neck", 13},
    {"Stopwatch", "stopwatch", "neck", 13},
};

// SubUrban / Hats (12)
static const ClothesDBItem g_clothes_s2_hats[] = {
    {"Cap Blue", "capblue", "cap", 16},
    {"Cap Blue Back", "capblueback", "capback", 16},
    {"Cap Blue Over", "capblueover", "capovereye", 16},
    {"Cap Blue Side", "capblueside", "capside", 16},
    {"Cap Blue Up", "capblueup", "caprimup", 16},
    {"Cap Red", "capred", "cap", 16},
    {"Cap Red Back", "capredback", "capback", 16},
    {"Cap Red Over", "capredover", "capovereye", 16},
    {"Cap Red Side", "capredside", "capside", 16},
    {"Cap Red Up", "capredup", "caprimup", 16},
    {"Skully Blk", "skullyblk", "skullycap", 16},
    {"Skully Grn", "skullygrn", "skullycap", 16},
};

// SubUrban / Torso (12)
static const ClothesDBItem g_clothes_s2_torso[] = {
    {"Coach", "coach", "coach", 0},
    {"Coachsemi", "coachsemi", "coach", 0},
    {"Hoody Abase 5", "hoodyabase5", "hoodya", 0},
    {"Hoody Amerc", "hoodyamerc", "hoodya", 0},
    {"Hoody Arock Star", "hoodyarockstar", "hoodya", 0},
    {"Sweatrstar", "sweatrstar", "sweat", 0},
    {"T-Shirt Base 5", "tshirtbase5", "tshirt", 0},
    {"T-Shirt Bobo Red", "tshirtbobored", "tshirt", 0},
    {"T-Shirt Bobomonk", "tshirtbobomonk", "tshirt", 0},
    {"T-Shirt Heat Wht", "tshirtheatwht", "tshirt", 0},
    {"T-Shirt Suburb", "tshirtsuburb", "tshirt", 0},
    {"W-Coat Blue", "wcoatblue", "wcoat", 0},
};

// SubUrban / Legs (8)
static const ClothesDBItem g_clothes_s2_legs[] = {
    {"Chonger Blue", "chongerblue", "chonger", 2},
    {"Chonger Gang", "chongergang", "chonger", 2},
    {"Chonger Grey", "chongergrey", "chonger", 2},
    {"Chonger Red", "chongerred", "chonger", 2},
    {"Denims Red", "denimsred", "jeans", 2},
    {"Shorts Gang", "shortsgang", "shorts", 2},
    {"Shorts Grey", "shortsgrey", "shorts", 2},
    {"Shorts Khaki", "shortskhaki", "shorts", 2},
};

// SubUrban / Shoes (8)
static const ClothesDBItem g_clothes_s2_shoes[] = {
    {"Bask 2 Heat Band", "bask2heatband", "bask1", 3},
    {"Bask 2 Heat Wht", "bask2heatwht", "bask1", 3},
    {"Conv Heat Blk", "convheatblk", "conv", 3},
    {"Conv Heat Orn", "convheatorn", "conv", 3},
    {"Conv Heat Red", "convheatred", "conv", 3},
    {"Sneaker Heat Blk", "sneakerheatblk", "sneaker", 3},
    {"Sneaker Heat Gry", "sneakerheatgry", "sneaker", 3},
    {"Sneaker Heat Wht", "sneakerheatwht", "sneaker", 3},
};

// SubUrban / Glasses (2)
static const ClothesDBItem g_clothes_s2_glasses[] = {
    {"Glasses 03 Blue", "glasses03blue", "glasses03", 15},
    {"Glasses 03 Red", "glasses03red", "glasses03", 15},
};

// SubUrban / Watches (2)
static const ClothesDBItem g_clothes_s2_watches[] = {
    {"Watch Sub 1", "watchsub1", "watch", 14},
    {"Watch Sub 2", "watchsub2", "watch", 14},
};

// SubUrban / Necklaces (2)
static const ClothesDBItem g_clothes_s2_necklaces[] = {
    {"Neck Ls", "neckls", "neck", 13},
    {"Neck Silver", "necksilver", "neck2", 13},
};

// Zip / Hats (7)
static const ClothesDBItem g_clothes_s3_hats[] = {
    {"Cap Zip", "capzip", "cap", 16},
    {"Cap Zip Back", "capzipback", "capback", 16},
    {"Cap Zip Over", "capzipover", "capovereye", 16},
    {"Cap Zip Side", "capzipside", "capside", 16},
    {"Cap Zip Up", "capzipup", "caprimup", 16},
    {"Hatmanc Blk", "hatmancblk", "hatmanc", 16},
    {"Hatmanc Plaid", "hatmancplaid", "hatmanc", 16},
};

// Zip / Torso (12)
static const ClothesDBItem g_clothes_s3_torso[] = {
    {"Bowling", "bowling", "hawaii", 0},
    {"Denim Fade", "denimfade", "denim", 0},
    {"Hoody A Black", "hoodyAblack", "hoodyA", 0},
    {"Hoody A Blue", "hoodyAblue", "hoodyA", 0},
    {"Shirt A Blue", "shirtablue", "shirta", 0},
    {"Shirt A Grey", "shirtagrey", "shirta", 0},
    {"Shirt A Yellow", "shirtayellow", "shirta", 0},
    {"Shirt B Plaid", "shirtbplaid", "shirtb", 0},
    {"Sleevtbrown", "sleevtbrown", "sleevt", 0},
    {"T-Shirt 2 Horiz", "tshirt2horiz", "tshirt2", 0},
    {"T-Shirt Zip Gry", "tshirtzipgry", "tshirt", 0},
    {"T-Shirt Zipcrm", "tshirtzipcrm", "tshirt", 0},
};

// Zip / Legs (6)
static const ClothesDBItem g_clothes_s3_legs[] = {
    {"Chinos Biege", "chinosbiege", "chinosb", 2},
    {"Chinos Black", "chinosblack", "chinosb", 2},
    {"Chinos Blue", "chinosblue", "chinosb", 2},
    {"Chinos Khaki", "chinoskhaki", "chinosb", 2},
    {"Cutoff Chinos", "cutoffchinos", "shorts", 2},
    {"Cutoff Chinos Blue", "cutoffchinosblue", "shorts", 2},
};

// Zip / Shoes (4)
static const ClothesDBItem g_clothes_s3_shoes[] = {
    {"Timber Fawn", "timberfawn", "bask1", 3},
    {"Timber Grey", "timbergrey", "bask1", 3},
    {"Timber Hike", "timberhike", "bask1", 3},
    {"Timber Red", "timberred", "bask1", 3},
};

// Zip / Glasses (2)
static const ClothesDBItem g_clothes_s3_glasses[] = {
    {"Glasses 01", "glasses01", "glasses01", 15},
    {"Glasses 04", "glasses04", "glasses04", 15},
};

// Zip / Watches (2)
static const ClothesDBItem g_clothes_s3_watches[] = {
    {"Watch Zip 1", "watchzip1", "watch", 14},
    {"Watch Zip 2", "watchzip2", "watch", 14},
};

// Zip / Necklaces (2)
static const ClothesDBItem g_clothes_s3_necklaces[] = {
    {"Neck Gold", "neckgold", "neck2", 13},
    {"Neck Hash", "neckhash", "neck", 13},
};

// Victim / Hats (7)
static const ClothesDBItem g_clothes_s4_hats[] = {
    {"Beret Blk", "beretblk", "beret", 16},
    {"Beret Red", "beretred", "beret", 16},
    {"Cap Blk", "capblk", "cap", 16},
    {"Cap Blk Back", "capblkback", "capback", 16},
    {"Cap Blk Over", "capblkover", "capovereye", 16},
    {"Cap Blk Side", "capblkside", "capside", 16},
    {"Cap Blk Up", "capblkup", "caprimup", 16},
};

// Victim / Torso (12)
static const ClothesDBItem g_clothes_s4_torso[] = {
    {"Bask Ballloc", "baskballloc", "baskball", 0},
    {"Hawaii Red", "hawaiired", "hawaii", 0},
    {"Hawaii Wht", "hawaiiwht", "hawaii", 0},
    {"Hoodjack Beige", "hoodjackbeige", "hoodjack", 0},
    {"Leather", "leather", "leather", 0},
    {"Painter", "painter", "painter", 0},
    {"Sport Jack", "sportjack", "trackytop1", 0},
    {"Suit 1 Blk", "suit1blk", "suit1", 0},
    {"Suit 1 Grey", "suit1grey", "suit1", 0},
    {"T-Shirt Loc Grey", "tshirtlocgrey", "tshirt", 0},
    {"T-Shirt Madd Grey", "tshirtmaddgrey", "tshirt", 0},
    {"T-Shirt Madd Grn", "tshirtmaddgrn", "tshirt", 0},
};

// Victim / Legs (5)
static const ClothesDBItem g_clothes_s4_legs[] = {
    {"Cutoff Denims", "cutoffdenims", "shorts", 2},
    {"Leathertr", "leathertr", "leathertr", 2},
    {"Leathertr Chaps", "leathertrchaps", "leathertr", 2},
    {"Suit 1 Tr Blk", "suit1trblk", "suit1tr", 2},
    {"Suit 1 Tr Grey", "suit1trgrey", "suit1tr", 2},
};

// Victim / Shoes (3)
static const ClothesDBItem g_clothes_s4_shoes[] = {
    {"Biker", "biker", "biker", 3},
    {"Cowboyboot", "cowboyboot", "biker", 3},
    {"Snakeskin", "snakeskin", "biker", 3},
};

// Victim / Glasses (2)
static const ClothesDBItem g_clothes_s4_glasses[] = {
    {"Glasses 03", "glasses03", "glasses03", 15},
    {"Glasses 03 Dark", "glasses03dark", "glasses03", 15},
};

// Victim / Watches (2)
static const ClothesDBItem g_clothes_s4_watches[] = {
    {"Watch Gno", "watchgno", "watch", 14},
    {"Watch Gno 2", "watchgno2", "watch", 14},
};

// Victim / Necklaces (2)
static const ClothesDBItem g_clothes_s4_necklaces[] = {
    {"Neck Ropeg", "neckropeg", "neck2", 13},
    {"Neck Ropes", "neckropes", "neck2", 13},
};

// Didier Sachs / Hats (9)
static const ClothesDBItem g_clothes_s5_hats[] = {
    {"Boater", "boater", "boater", 16},
    {"Boater Blk", "boaterblk", "boater", 16},
    {"Bowler", "bowler", "bowler", 16},
    {"Bowler Blue", "bowlerblue", "bowler", 16},
    {"Bowler Gang", "bowlergang", "bowler", 16},
    {"Bowler Red", "bowlerred", "bowler", 16},
    {"Bowler Yellow", "bowleryellow", "bowler", 16},
    {"Trilby Drk", "trilbydrk", "trilby", 16},
    {"Trilby Lght", "trilbylght", "trilby", 16},
};

// Didier Sachs / Torso (7)
static const ClothesDBItem g_clothes_s5_torso[] = {
    {"Letter", "letter", "sleevt", 0},
    {"Suit 1 Blue", "suit1blue", "suit1", 0},
    {"Suit 1 Gang", "suit1gang", "suit1", 0},
    {"Suit 1 Red", "suit1red", "suit1", 0},
    {"Suit 1 Yellow", "suit1yellow", "suit1", 0},
    {"Suit 2 Grn", "suit2grn", "suit2", 0},
    {"Tuxedo", "tuxedo", "suit2", 0},
};

// Didier Sachs / Legs (6)
static const ClothesDBItem g_clothes_s5_legs[] = {
    {"Suit 1 Tr Blk 2", "suit1trblk2", "suit1tr", 2},
    {"Suit 1 Tr Blue", "suit1trblue", "suit1tr", 2},
    {"Suit 1 Tr Green", "suit1trgreen", "suit1tr", 2},
    {"Suit 1 Tr Red", "suit1trred", "suit1tr", 2},
    {"Suit 1 Tr Yellow", "suit1tryellow", "suit1tr", 2},
    {"Suit 1 Trgang", "suit1trgang", "suit1tr", 2},
};

// Didier Sachs / Shoes (3)
static const ClothesDBItem g_clothes_s5_shoes[] = {
    {"Shoedress Blk", "shoedressblk", "shoe", 3},
    {"Shoedress Brn", "shoedressbrn", "shoe", 3},
    {"Shoespatz", "shoespatz", "shoe", 3},
};

// Didier Sachs / Glasses (2)
static const ClothesDBItem g_clothes_s5_glasses[] = {
    {"Glasses 05", "glasses05", "glasses03", 15},
    {"Glasses 05 Dark", "glasses05dark", "glasses03", 15},
};

// Didier Sachs / Watches (2)
static const ClothesDBItem g_clothes_s5_watches[] = {
    {"Watch Cro", "watchcro", "watch", 14},
    {"Watch Cro 2", "watchcro2", "watch", 14},
};

// Didier Sachs / Necklaces (2)
static const ClothesDBItem g_clothes_s5_necklaces[] = {
    {"Neck Cross", "neckcross", "neck", 13},
    {"Neck Dollar", "neckdollar", "neck", 13},
};

// [store][category] lookup
static const ClothesDBCat g_clothes_db[6][7] = {
    { // Binco
        {g_clothes_s0_hats, (int)(sizeof(g_clothes_s0_hats)/sizeof(g_clothes_s0_hats[0]))}, // Hats
        {g_clothes_s0_torso, (int)(sizeof(g_clothes_s0_torso)/sizeof(g_clothes_s0_torso[0]))}, // Torso
        {g_clothes_s0_legs, (int)(sizeof(g_clothes_s0_legs)/sizeof(g_clothes_s0_legs[0]))}, // Legs
        {g_clothes_s0_shoes, (int)(sizeof(g_clothes_s0_shoes)/sizeof(g_clothes_s0_shoes[0]))}, // Shoes
        {g_clothes_s0_glasses, (int)(sizeof(g_clothes_s0_glasses)/sizeof(g_clothes_s0_glasses[0]))}, // Glasses
        {g_clothes_s0_watches, (int)(sizeof(g_clothes_s0_watches)/sizeof(g_clothes_s0_watches[0]))}, // Watches
        {g_clothes_s0_necklaces, (int)(sizeof(g_clothes_s0_necklaces)/sizeof(g_clothes_s0_necklaces[0]))}, // Necklaces
    },
    { // ProLaps
        {g_clothes_s1_hats, (int)(sizeof(g_clothes_s1_hats)/sizeof(g_clothes_s1_hats[0]))}, // Hats
        {g_clothes_s1_torso, (int)(sizeof(g_clothes_s1_torso)/sizeof(g_clothes_s1_torso[0]))}, // Torso
        {g_clothes_s1_legs, (int)(sizeof(g_clothes_s1_legs)/sizeof(g_clothes_s1_legs[0]))}, // Legs
        {g_clothes_s1_shoes, (int)(sizeof(g_clothes_s1_shoes)/sizeof(g_clothes_s1_shoes[0]))}, // Shoes
        {g_clothes_s1_glasses, (int)(sizeof(g_clothes_s1_glasses)/sizeof(g_clothes_s1_glasses[0]))}, // Glasses
        {g_clothes_s1_watches, (int)(sizeof(g_clothes_s1_watches)/sizeof(g_clothes_s1_watches[0]))}, // Watches
        {g_clothes_s1_necklaces, (int)(sizeof(g_clothes_s1_necklaces)/sizeof(g_clothes_s1_necklaces[0]))}, // Necklaces
    },
    { // SubUrban
        {g_clothes_s2_hats, (int)(sizeof(g_clothes_s2_hats)/sizeof(g_clothes_s2_hats[0]))}, // Hats
        {g_clothes_s2_torso, (int)(sizeof(g_clothes_s2_torso)/sizeof(g_clothes_s2_torso[0]))}, // Torso
        {g_clothes_s2_legs, (int)(sizeof(g_clothes_s2_legs)/sizeof(g_clothes_s2_legs[0]))}, // Legs
        {g_clothes_s2_shoes, (int)(sizeof(g_clothes_s2_shoes)/sizeof(g_clothes_s2_shoes[0]))}, // Shoes
        {g_clothes_s2_glasses, (int)(sizeof(g_clothes_s2_glasses)/sizeof(g_clothes_s2_glasses[0]))}, // Glasses
        {g_clothes_s2_watches, (int)(sizeof(g_clothes_s2_watches)/sizeof(g_clothes_s2_watches[0]))}, // Watches
        {g_clothes_s2_necklaces, (int)(sizeof(g_clothes_s2_necklaces)/sizeof(g_clothes_s2_necklaces[0]))}, // Necklaces
    },
    { // Zip
        {g_clothes_s3_hats, (int)(sizeof(g_clothes_s3_hats)/sizeof(g_clothes_s3_hats[0]))}, // Hats
        {g_clothes_s3_torso, (int)(sizeof(g_clothes_s3_torso)/sizeof(g_clothes_s3_torso[0]))}, // Torso
        {g_clothes_s3_legs, (int)(sizeof(g_clothes_s3_legs)/sizeof(g_clothes_s3_legs[0]))}, // Legs
        {g_clothes_s3_shoes, (int)(sizeof(g_clothes_s3_shoes)/sizeof(g_clothes_s3_shoes[0]))}, // Shoes
        {g_clothes_s3_glasses, (int)(sizeof(g_clothes_s3_glasses)/sizeof(g_clothes_s3_glasses[0]))}, // Glasses
        {g_clothes_s3_watches, (int)(sizeof(g_clothes_s3_watches)/sizeof(g_clothes_s3_watches[0]))}, // Watches
        {g_clothes_s3_necklaces, (int)(sizeof(g_clothes_s3_necklaces)/sizeof(g_clothes_s3_necklaces[0]))}, // Necklaces
    },
    { // Victim
        {g_clothes_s4_hats, (int)(sizeof(g_clothes_s4_hats)/sizeof(g_clothes_s4_hats[0]))}, // Hats
        {g_clothes_s4_torso, (int)(sizeof(g_clothes_s4_torso)/sizeof(g_clothes_s4_torso[0]))}, // Torso
        {g_clothes_s4_legs, (int)(sizeof(g_clothes_s4_legs)/sizeof(g_clothes_s4_legs[0]))}, // Legs
        {g_clothes_s4_shoes, (int)(sizeof(g_clothes_s4_shoes)/sizeof(g_clothes_s4_shoes[0]))}, // Shoes
        {g_clothes_s4_glasses, (int)(sizeof(g_clothes_s4_glasses)/sizeof(g_clothes_s4_glasses[0]))}, // Glasses
        {g_clothes_s4_watches, (int)(sizeof(g_clothes_s4_watches)/sizeof(g_clothes_s4_watches[0]))}, // Watches
        {g_clothes_s4_necklaces, (int)(sizeof(g_clothes_s4_necklaces)/sizeof(g_clothes_s4_necklaces[0]))}, // Necklaces
    },
    { // Didier Sachs
        {g_clothes_s5_hats, (int)(sizeof(g_clothes_s5_hats)/sizeof(g_clothes_s5_hats[0]))}, // Hats
        {g_clothes_s5_torso, (int)(sizeof(g_clothes_s5_torso)/sizeof(g_clothes_s5_torso[0]))}, // Torso
        {g_clothes_s5_legs, (int)(sizeof(g_clothes_s5_legs)/sizeof(g_clothes_s5_legs[0]))}, // Legs
        {g_clothes_s5_shoes, (int)(sizeof(g_clothes_s5_shoes)/sizeof(g_clothes_s5_shoes[0]))}, // Shoes
        {g_clothes_s5_glasses, (int)(sizeof(g_clothes_s5_glasses)/sizeof(g_clothes_s5_glasses[0]))}, // Glasses
        {g_clothes_s5_watches, (int)(sizeof(g_clothes_s5_watches)/sizeof(g_clothes_s5_watches[0]))}, // Watches
        {g_clothes_s5_necklaces, (int)(sizeof(g_clothes_s5_necklaces)/sizeof(g_clothes_s5_necklaces[0]))}, // Necklaces
    },
};

static const char* g_clothes_stores[6] = {
    "Binco",
    "ProLaps",
    "SubUrban",
    "Zip",
    "Victim",
    "Didier Sachs",
};

static const char* g_clothes_cats[7] = {"Hats","Torso","Legs","Shoes","Glasses","Watches","Necklaces"};
