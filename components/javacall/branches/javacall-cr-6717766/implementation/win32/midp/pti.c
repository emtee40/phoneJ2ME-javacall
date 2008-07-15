/*
 *
 * Copyright  1990-2007 Sun Microsystems, Inc. All Rights Reserved.
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER
 * 
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License version
 * 2 only, as published by the Free Software Foundation.
 * 
 * This program is distributed in the hope that it will be useful, but
 * WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU
 * General Public License version 2 for more details (a copy is
 * included at /legal/license.txt).
 * 
 * You should have received a copy of the GNU General Public License
 * version 2 along with this work; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA
 * 02110-1301 USA
 * 
 * Please contact Sun Microsystems, Inc., 4150 Network Circle, Santa
 * Clara, CA 95054 or visit www.sun.com if you need additional
 * information or have any questions.
 */ 

#include "javacall_pti.h"
#include "javacall_memory.h"
#include "javacall_memory.h"
#include <string.h>

#define keysMaxNum 10
#define englWordListLength 1797
char* englWordList[englWordListLength]; //most useful english words list
char* keyMap[10] = {"0", "1", "2abc", "3def", "4ghi", "5jkl", "6mno", "7pqrs", "8tuv", "9wxyz"}; //phone key values

typedef struct {
    int englWordListPos;
    int keysNum;
    javacall_pti_keycode keys[keysMaxNum];
} pti_state;

/**
 * helper func
 * returns 1 on success, 0 otherwise
 */
static int isCharMatchKeycode(char ch, javacall_pti_keycode keykode) {
    char* chars = keyMap[keykode - JAVACALL_PTI_KEYCODE_0];
    return (strchr(chars, ch) != NULL);
}

/**
 * helper func
 * returns 1 on success, 0 otherwise
 */
static int isWordMatchKeycodes(const char* pWord, pti_state* state) {
    int i;
    for (i=0; i<state->keysNum; i++) {
        if (*pWord == 0) {
            return 0;
        }
        if (!isCharMatchKeycode(*pWord, state->keys[i])) {
            return 0;
        }
        pWord++;
    }
    return 1;
}

/**
 * helper func
 * finds word in englWordList:
 *    beginning from state->position + 1
 *    matching state->keys
 */
static int findNextMatch(pti_state* state) {
    int i;
    for (i = state->englWordListPos + 1; i < englWordListLength; i++) {
        if (isWordMatchKeycodes(englWordList[i], state)) {
            return i;
        }
    }
    return -1;
}

/**
 * Called 1st time PTI library is accessed.
 * This function may perform specific PTI initialization functions
 * @retval JAVACALL_OK if support is available
 * @retval JAVACALL_FAIL on error
 */
javacall_result javacall_pti_init(void) {
    return JAVACALL_OK;
}


/**
 * create a new PTI iterator instance 
 * Language dictionary must be set default locale language
 *
 * @return handle of new PTI iterator or <tt>0</tt> on error
 */
javacall_handle javacall_pti_open(void) {
    pti_state* state = javacall_malloc(sizeof(pti_state));
    if (state == NULL) {
        return 0;
    }
    state->englWordListPos = -1;
    state->keysNum = 0;
    return state;
}

/**
 * delete a PTI iterator. 
 *
 * @retval JAVACALL_OK if successful
 * @retval JAVACALL_FAIL on error
 */
javacall_result javacall_pti_close(javacall_handle handle) {
    javacall_free((pti_state*)handle);
    return JAVACALL_OK;
}

/**
 * Set a dictionary for a PTI iterator. 
 * All newly created iterators are defaultly set to defualt locale language
 * this function is called explicitly
 *
 * @param handle the handle of the iterator 
 * @param dictionary the dictionary for the text prediction
 *
 * @retval JAVACALL_OK if language is supported
 * @retval JAVACALL_FAIL on error
 */
javacall_result javacall_pti_set_dictionary(javacall_handle handle, 
                                           javacall_pti_dictionary dictionary) {

    return (dictionary == JAVACALL_PTI_DICTIONARY_ENGLISH) ? JAVACALL_OK : JAVACALL_FAIL;
}


/**
 * advance PTI iterator using the next key code
 *
 * @param handle the handle of the iterator 
 * @param keyCode the next key (representing one of the keys '0'-'9')
 *
 * @return JAVACALL_OK if successful, or JAVACALL_FAIL on error
 */
javacall_result javacall_pti_add_key(javacall_handle handle, 
                                    javacall_pti_keycode keyCode) {

    pti_state* state = (pti_state*)handle;
    if (state->keysNum >= keysMaxNum) {
        return JAVACALL_FAIL;
    }

    state->keys[state->keysNum] = keyCode;
    state->keysNum++;
    
    return JAVACALL_OK;
}

/**
 * Backspace the iterator one key 
 * If current string is empty, has no effect.
 *
 * @param handle the handle of the iterator 
 * @return JAVACALL_OK if successful, or JAVACALL_FAIL on error
 */
javacall_result javacall_pti_backspace(javacall_handle handle) {

    pti_state* state = (pti_state*)handle;

    if (state->keysNum == 0) {
        return JAVACALL_FAIL;
    }

    state->keysNum--;
    return JAVACALL_OK;
}

/**
 * Clear all text from the PTI iterator 
 *
 * @param handle the handle of the iterator 
 * @return JAVACALL_OK if successful, or JAVACALL_FAIL on error
 */
javacall_result javacall_pti_clear_all(javacall_handle handle) {

    pti_state* state = (pti_state*)handle;
    state->keysNum = 0;
    state->englWordListPos = -1;

    return JAVACALL_OK;
}

static unsigned char_to_unicode(javacall_utf16* unicode_name, unsigned unicode_name_len, char* str) {

    unsigned i;
    for (i = 0; (i < (unsigned)strlen(str)+1) && (i < unicode_name_len); i++) {
        unicode_name[i] = (javacall_utf16) str[i];
    }
    return i-1;
}


/**
 * return the current PTI completion option
 *
 * @param handle the handle of the iterator 
 * @param outArray the array to be filled with UNICODE string 
 * @param outStringLen max size of the outArray 
 *
 * @return number of chars returned if successful, or <tt>0</tt> otherwise 
 */
int javacall_pti_completion_get_next(
                    javacall_handle     handle, 
                    javacall_utf16*   outString, 
                    int                 outStringLen) {

    pti_state* state = (pti_state*)handle;

    int englWordListNum = findNextMatch(state);
    state->englWordListPos = englWordListNum;

    if (englWordListNum == -1) {
        return 0;
    } else {
        return char_to_unicode(outString, outStringLen, englWordList[englWordListNum]);
    }
}


/**
 * see if further completion options exist for the current PTI entry
 *
 * @param handle the handle of the iterator 
 *
 * @retval JAVACALL_OK if more completion options exist
 * @retval JAVACALL_FAIL if no more completion options exist
 */
javacall_result javacall_pti_completion_has_next(javacall_handle handle) {

    pti_state* state = (pti_state*)handle;
    int englWordListNum = findNextMatch(state);

    return (englWordListNum == -1) ? JAVACALL_FAIL : JAVACALL_OK;
}

/**
 * reset completion options for for the current PTI entry
 * After this call, the function javacall_pti_completion_get_next() will 
 * return all completion options starting from 1st option
 *
 * @param handle the handle of the iterator 
 * @return JAVACALL_OK  if successful, JAVACALL_FAIL otherwise 
 */
javacall_result javacall_pti_completion_rewind(javacall_handle handle) {

    pti_state* state = (pti_state*)handle;
    state->englWordListPos = -1;

    return JAVACALL_OK;
}

char* englWordList[englWordListLength] = {
"a",
"ability",
"able",
"be",
"about",
"above",
"absence",
"absolutely",
"academic",
"accept",
"access",
"accident",
"accompany",
"according",
"account",
"achieve",
"achievement",
"acid",
"acquire",
"across",
"act",
"action",
"active",
"activity",
"actual",
"actually",
"add",
"addition",
"additional",
"address",
"administration",
"admit",
"adopt",
"adult",
"advance",
"advantage",
"advice",
"advise",
"affair",
"affect",
"afford",
"afraid",
"after",
"afternoon",
"afterwards",
"again",
"against",
"age",
"agency",
"agent",
"ago",
"agree",
"agreement",
"ahead",
"aid",
"aim",
"air",
"aircraft",
"all",
"allow",
"almost",
"alone",
"along",
"already",
"alright",
"also",
"alternative",
"although",
"always",
"among",
"amongst",
"amount",
"analysis",
"ancient",
"and",
"animal",
"announce",
"annual",
"another",
"answer",
"any",
"anybody",
"anyone",
"anything",
"anyway",
"apart",
"apparent",
"apparently",
"appeal",
"appear",
"appearance",
"application",
"apply",
"appoint",
"appointment",
"approach",
"appropriate",
"approve",
"area",
"argue",
"argument",
"arise",
"arm",
"army",
"around",
"arrange",
"arrangement",
"arrive",
"art",
"article",
"artist",
"as",
"ask",
"aspect",
"assembly",
"assess",
"assessment",
"asset",
"associate",
"association",
"assume",
"assumption",
"at",
"atmosphere",
"attach",
"attack",
"attempt",
"attend",
"attention",
"attitude",
"attract",
"attractive",
"audience",
"author",
"authority",
"available",
"average",
"avoid",
"award",
"aware",
"away",
"baby",
"back",
"background",
"bad",
"bag",
"balance",
"ball",
"band",
"bank",
"bar",
"base",
"basic",
"basis",
"battle",
"be",
"bear",
"beat",
"beautiful",
"because",
"become",
"bed",
"bedroom",
"before",
"begin",
"beginning",
"behaviour",
"behind",
"belief",
"believe",
"belong",
"below",
"beneath",
"benefit",
"beside",
"best",
"better",
"between",
"beyond",
"big",
"bill",
"bind",
"bird",
"birth",
"bit",
"black",
"block",
"blood",
"bloody",
"blow",
"blue",
"board",
"boat",
"body",
"bone",
"book",
"border",
"both",
"bottle",
"bottom",
"box",
"boy",
"brain",
"branch",
"break",
"breath",
"bridge",
"brief",
"bright",
"bring",
"broad",
"brother",
"budget",
"build",
"building",
"burn",
"bus",
"business",
"busy",
"but",
"buy",
"by",
"cabinet",
"call",
"campaign",
"cdo",
"candidate",
"capable",
"capacity",
"capital",
"car",
"card",
"care",
"career",
"careful",
"carefully",
"carry",
"case",
"cash",
"cat",
"catch",
"category",
"cause",
"cell",
"central",
"centre",
"century",
"certain",
"certainly",
"chain",
"chair",
"chairman",
"challenge",
"chance",
"change",
"channel",
"chapter",
"character",
"characteristic",
"charge",
"cheap",
"check",
"chemical",
"chief",
"child",
"choice",
"choose",
"church",
"circle",
"circumstance",
"citizen",
"city",
"civil",
"claim",
"class",
"clean",
"clear",
"clearly",
"client",
"climb",
"be",
"close",
"closely",
"clothes",
"club",
"coal",
"code",
"coffee",
"cold",
"colleague",
"collect",
"collection",
"college",
"colour",
"combination",
"combine",
"come",
"comment",
"commercial",
"commission",
"commit",
"commitment",
"committee",
"common",
"communication",
"community",
"company",
"compare",
"comparison",
"competition",
"complete",
"completely",
"complex",
"component",
"computer",
"concentrate",
"concentration",
"concept",
"concern",
"concerned",
"conclude",
"conclusion",
"condition",
"conduct",
"conference",
"confidence",
"confirm",
"conflict",
"congress",
"connect",
"connection",
"consequence",
"conservative",
"consider",
"considerable",
"consideration",
"consist",
"constant",
"construction",
"consumer",
"contact",
"contain",
"content",
"context",
"continue",
"contract",
"contrast",
"contribute",
"contribution",
"control",
"convention",
"conversation",
"copy",
"corner",
"corporate",
"correct",
"'cos",
"cost",
"could",
"council",
"count",
"country",
"county",
"couple",
"course",
"court",
"cover",
"create",
"creation",
"credit",
"crime",
"criminal",
"crisis",
"criterion",
"critical",
"criticism",
"cross",
"crowd",
"cry",
"cultural",
"culture",
"cup",
"current",
"currently",
"curriculum",
"customer",
"cut",
"damage",
"danger",
"dangerous",
"dark",
"data",
"date",
"daughter",
"day",
"dead",
"deal",
"death",
"debate",
"debt",
"decade",
"decide",
"decision",
"declare",
"deep",
"defence",
"defense",
"defendant",
"define",
"definition",
"degree",
"deliver",
"demand",
"democratic",
"demonstrate",
"deny",
"department",
"depend",
"deputy",
"derive",
"describe",
"description",
"design",
"desire",
"desk",
"despite",
"destroy",
"detail",
"detailed",
"determine",
"develop",
"development",
"device",
"die",
"difference",
"different",
"difficult",
"difficulty",
"dinner",
"direct",
"direction",
"directly",
"director",
"disappear",
"discipline",
"discover",
"discuss",
"discussion",
"disease",
"display",
"distance",
"distinction",
"distribution",
"district",
"divide",
"division",
"do",
"doctor",
"document",
"dog",
"domestic",
"door",
"double",
"doubt",
"down",
"draw",
"drawing",
"dream",
"dress",
"drink",
"drive",
"driver",
"drop",
"drug",
"dry",
"due",
"during",
"duty",
"each",
"ear",
"early",
"earn",
"earth",
"easily",
"east",
"easy",
"eat",
"economic",
"economy",
"edge",
"editor",
"education",
"educational",
"effect",
"effective",
"effectively",
"effort",
"egg",
"either",
"elderly",
"election",
"element",
"else",
"elsewhere",
"emerge",
"emphasis",
"employ",
"employee",
"employer",
"employment",
"empty",
"enable",
"encourage",
"end",
"enemy",
"energy",
"engine",
"engineering",
"enjoy",
"enough",
"ensure",
"enter",
"enterprise",
"entire",
"entirely",
"entitle",
"entry",
"environment",
"environmental",
"equal",
"equally",
"equipment",
"error",
"escape",
"especially",
"essential",
"establish",
"establishment",
"estate",
"estimate",
"even",
"evening",
"event",
"eventually",
"ever",
"every",
"everybody",
"everyone",
"everything",
"evidence",
"exactly",
"examination",
"examine",
"example",
"excellent",
"except",
"exchange",
"executive",
"exercise",
"exhibition",
"exist",
"existence",
"existing",
"expect",
"expectation",
"expenditure",
"expense",
"expensive",
"experience",
"experiment",
"expert",
"explain",
"explanation",
"explore",
"express",
"expression",
"extend",
"extent",
"external",
"extra",
"extremely",
"eye",
"face",
"facility",
"fact",
"factor",
"factory",
"fail",
"failure",
"fair",
"fairly",
"faith",
"fall",
"familiar",
"family",
"famous",
"far",
"farm",
"farmer",
"fashion",
"fast",
"father",
"favour",
"fear",
"feature",
"fee",
"feel",
"feeling",
"female",
"few",
"field",
"fight",
"figure",
"file",
"fill",
"film",
"final",
"finally",
"finance",
"financial",
"find",
"finding",
"fine",
"finger",
"finish",
"fire",
"firm",
"first",
"fish",
"fit",
"fix",
"flat",
"flight",
"floor",
"flow",
"flower",
"fly",
"focus",
"follow",
"following",
"food",
"foot",
"football",
"for",
"force",
"foreign",
"forest",
"forget",
"form",
"formal",
"former",
"forward",
"foundation",
"free",
"freedom",
"frequently",
"fresh",
"friend",
"from",
"front",
"the",
"fruit",
"fuel",
"full",
"fully",
"function",
"fund",
"funny",
"further",
"future",
"gain",
"game",
"garden",
"gas",
"gate",
"gather",
"general",
"generally",
"generate",
"generation",
"gentleman",
"get",
"girl",
"give",
"glass",
"go",
"goal",
"god",
"gold",
"good",
"goods",
"government",
"grant",
"great",
"green",
"grey",
"ground",
"group",
"grow",
"growing",
"growth",
"guest",
"guide",
"gun",
"hair",
"half",
"hall",
"hand",
"handle",
"hang",
"happen",
"happy",
"hard",
"hardly",
"hate",
"have",
"he",
"head",
"health",
"chear",
"hear",
"heart",
"heat",
"heavy",
"hell",
"help",
"hence",
"her",
"here",
"herself",
"hide",
"high",
"highly",
"hill",
"him",
"himself",
"his",
"historical",
"history",
"hit",
"hold",
"hole",
"holiday",
"home",
"hope",
"horse",
"hospital",
"hot",
"hotel",
"hour",
"house",
"household",
"housing",
"how",
"however",
"huge",
"human",
"hurt",
"husband",
"I",
"idea",
"identify",
"if",
"ignore",
"illustrate",
"image",
"imagine",
"immediate",
"immediately",
"impact",
"implication",
"imply",
"importance",
"important",
"impose",
"impossible",
"impression",
"improve",
"improvement",
"in",
"incident",
"include",
"including",
"income",
"increase",
"increased",
"increasingly",
"indeed",
"independent",
"index",
"indicate",
"individual",
"industrial",
"industry",
"influence",
"inform",
"information",
"initial",
"initiative",
"injury",
"inside",
"insist",
"instance",
"instead",
"institute",
"institution",
"instruction",
"instrument",
"insurance",
"intend",
"intention",
"interest",
"interested",
"interesting",
"internal",
"international",
"interpretation",
"interview",
"into",
"introduce",
"introduction",
"investigate",
"investigation",
"investment",
"invite",
"involve",
"iron",
"island",
"issue",
"it",
"item",
"its",
"itself",
"job",
"join",
"joint",
"journey",
"judge",
"jump",
"just",
"justice",
"keep",
"key",
"kid",
"kill",
"kind",
"king",
"kitchen",
"knee",
"know",
"knowledge",
"labour",
"lack",
"lady",
"land",
"language",
"large",
"largely",
"last",
"late",
"later",
"latter",
"laugh",
"launch",
"law",
"lawyer",
"lay",
"lead",
"leader",
"leadership",
"leading",
"leaf",
"league",
"lean",
"learn",
"least",
"leave",
"left",
"leg",
"legal",
"legislation",
"length",
"less",
"let",
"letter",
"level",
"liability",
"liberal",
"library",
"lie",
"life",
"lift",
"light",
"like",
"be",
"like",
"likely",
"limit",
"limited",
"line",
"link",
"lip",
"list",
"listen",
"literature",
"little",
"live",
"living",
"loan",
"local",
"location",
"long",
"look",
"lord",
"lose",
"loss",
"lot",
"love",
"lovely",
"low",
"lunch",
"machine",
"magazine",
"main",
"mainly",
"maintain",
"major",
"majority",
"make",
"be",
"male",
"man",
"manage",
"management",
"manager",
"manner",
"many",
"map",
"mark",
"market",
"marriage",
"be",
"get",
"marry",
"mass",
"master",
"match",
"material",
"matter",
"may",
"maybe",
"me",
"meal",
"mesthg",
"meaning",
"means",
"meanwhile",
"measure",
"mechanism",
"media",
"medical",
"meet",
"meeting",
"member",
"membership",
"memory",
"mental",
"mention",
"merely",
"message",
"metal",
"method",
"middle",
"might",
"mile",
"military",
"milk",
"mind",
"mine",
"minister",
"ministry",
"minute",
"miss",
"mistake",
"model",
"modern",
"module",
"moment",
"money",
"month",
"more",
"morning",
"most",
"mother",
"motion",
"motor",
"mountain",
"mouth",
"move",
"movement",
"much",
"murder",
"museum",
"music",
"must",
"my",
"myself",
"name",
"narrow",
"nation",
"national",
"natural",
"nature",
"near",
"nearly",
"necessarily",
"necessary",
"neck",
"need",
"negotiation",
"neighbour",
"neither",
"network",
"never",
"nevertheless",
"new",
"news",
"newspaper",
"next",
"nice",
"night",
"no",
"no-one",
"nobody",
"nod",
"noise",
"none",
"nor",
"normal",
"normally",
"north",
"northern",
"nose",
"not",
"note",
"nothing",
"notice",
"notion",
"now",
"nuclear",
"number",
"nurse",
"object",
"objective",
"observation",
"observe",
"obtain",
"obvious",
"obviously",
"occasion",
"occur",
"odd",
"of",
"off",
"offence",
"offer",
"office",
"officer",
"official",
"often",
"oil",
"okay",
"old",
"on",
"once",
"one",
"only",
"onto",
"be",
"open",
"operate",
"operation",
"opinion",
"opportunity",
"opposition",
"option",
"or",
"order",
"ordinary",
"organisation",
"organise",
"organization",
"origin",
"original",
"other",
"otherwise",
"ought",
"our",
"ourselves",
"out",
"outcome",
"output",
"outside",
"over",
"overall",
"own",
"owner",
"package",
"page",
"pain",
"paint",
"painting",
"pair",
"panel",
"paper",
"parent",
"park",
"parliament",
"part",
"particular",
"particularly",
"partly",
"partner",
"party",
"pass",
"passage",
"past",
"path",
"patient",
"pattern",
"pay",
"payment",
"peace",
"pension",
"people",
"per",
"percent",
"perfect",
"perform",
"performance",
"perhaps",
"period",
"permanent",
"person",
"personal",
"persuade",
"phase",
"phone",
"photograph",
"physical",
"pick",
"picture",
"piece",
"place",
"plan",
"plsthg",
"planning",
"plant",
"plastic",
"plate",
"play",
"player",
"please",
"pleasure",
"plenty",
"plus",
"pocket",
"point",
"the",
"policy",
"political",
"politics",
"pool",
"poor",
"popular",
"population",
"position",
"positive",
"possibility",
"possible",
"possibly",
"post",
"potential",
"pound",
"power",
"powerful",
"practical",
"practice",
"prefer",
"prepare",
"presence",
"present",
"president",
"press",
"pressure",
"pretty",
"prevent",
"previous",
"previously",
"price",
"primary",
"prime",
"principle",
"priority",
"prison",
"prisoner",
"private",
"probably",
"problem",
"procedure",
"process",
"produce",
"product",
"production",
"professional",
"profit",
"program",
"programme",
"progress",
"project",
"promise",
"promote",
"proper",
"properly",
"property",
"proportion",
"propose",
"proposal",
"prospect",
"protect",
"protection",
"prove",
"provide",
"provided",
"provision",
"pub",
"public",
"publication",
"publish",
"pull",
"pupil",
"purpose",
"push",
"put",
"quality",
"quarter",
"question",
"quick",
"quickly",
"quiet",
"quite",
"race",
"radio",
"railway",
"rain",
"raise",
"range",
"rapidly",
"rare",
"rate",
"rather",
"reach",
"reaction",
"read",
"reader",
"reading",
"ready",
"real",
"realise",
"reality",
"realize",
"really",
"reason",
"reasonable",
"recall",
"receive",
"recent",
"recently",
"recognise",
"recognition",
"recognize",
"recommend",
"record",
"recover",
"red",
"reduce",
"reduction",
"refer",
"reference",
"reflect",
"reform",
"refuse",
"regard",
"region",
"regional",
"regular",
"regulation",
"reject",
"relate",
"relation",
"relationship",
"relative",
"relatively",
"release",
"relevant",
"relief",
"religion",
"religious",
"rely",
"remain",
"remember",
"remind",
"remove",
"repeat",
"replace",
"reply",
"report",
"represent",
"representation",
"representative",
"request",
"require",
"requirement",
"research",
"resource",
"respect",
"respond",
"response",
"responsibility",
"responsible",
"rest",
"restaurant",
"result",
"retain",
"return",
"reveal",
"revenue",
"review",
"revolution",
"rich",
"ride",
"right",
"ring",
"rise",
"risk",
"river",
"road",
"rock",
"role",
"roll",
"roof",
"room",
"round",
"route",
"row",
"royal",
"rule",
"run",
"rural",
"safe",
"safety",
"sale",
"same",
"sample",
"satisfy",
"save",
"say",
"scale",
"scene",
"scheme",
"school",
"science",
"scientific",
"scientist",
"score",
"screen",
"sea",
"search",
"season",
"seat",
"second",
"secondary",
"secretary",
"section",
"sector",
"secure",
"security",
"csee",
"see",
"seek",
"seem",
"select",
"selection",
"sell",
"send",
"senior",
"sense",
"sentence",
"separate",
"sequence",
"series",
"serious",
"seriously",
"servant",
"serve",
"service",
"session",
"set",
"settle",
"settlement",
"several",
"severe",
"sex",
"sexual",
"shake",
"shall",
"shape",
"share",
"she",
"sheet",
"ship",
"shoe",
"shoot",
"shop",
"short",
"shot",
"should",
"shoulder",
"shout",
"show",
"shut",
"side",
"sight",
"sign",
"signal",
"significance",
"significant",
"silence",
"similar",
"simple",
"simply",
"since",
"sing",
"single",
"sir",
"sister",
"sit",
"site",
"situation",
"size",
"skill",
"skin",
"sky",
"sleep",
"slightly",
"slip",
"slow",
"slowly",
"small",
"smile",
"so",
"social",
"society",
"soft",
"software",
"soil",
"soldier",
"solicitor",
"solution",
"some",
"somebody",
"someone",
"something",
"sometimes",
"somewhat",
"somewhere",
"son",
"song",
"soon",
"be",
"sort",
"sound",
"source",
"south",
"southern",
"space",
"speak",
"speaker",
"special",
"species",
"specific",
"speech",
"speed",
"spend",
"spirit",
"sport",
"spot",
"spread",
"spring",
"staff",
"stage",
"stand",
"standard",
"star",
"start",
"state",
"statement",
"station",
"status",
"stay",
"steal",
"step",
"stick",
"still",
"stock",
"stone",
"stop",
"store",
"story",
"straight",
"strange",
"strategy",
"street",
"strength",
"strike",
"strong",
"strongly",
"structure",
"student",
"studio",
"study",
"stuff",
"style",
"subject",
"substantial",
"succeed",
"success",
"successful",
"such",
"suddenly",
"suffer",
"sufficient",
"suggest",
"suggestion",
"suitable",
"sum",
"summer",
"sun",
"supply",
"support",
"suppose",
"sure",
"surely",
"surface",
"surprise",
"surround",
"survey",
"survive",
"switch",
"system",
"table",
"take",
"talk",
"tall",
"tape",
"target",
"task",
"tax",
"tea",
"teach",
"teacher",
"teaching",
"team",
"tear",
"technical",
"technique",
"technology",
"telephone",
"television",
"tell",
"temperature",
"tend",
"term",
"terms",
"terrible",
"test",
"text",
"than",
"thank",
"thanks",
"that",
"the",
"theatre",
"their",
"them",
"theme",
"themselves",
"then",
"theory",
"there",
"therefore",
"these",
"they",
"thin",
"thing",
"think",
"this",
"those",
"though",
"thought",
"threat",
"threaten",
"through",
"throughout",
"throw",
"thus",
"ticket",
"time",
"tiny",
"title",
"to",
"today",
"together",
"tomorrow",
"tone",
"tonight",
"too",
"tool",
"tooth",
"top",
"total",
"totally",
"touch",
"tour",
"towards",
"town",
"track",
"trade",
"tradition",
"traditional",
"traffic",
"train",
"training",
"transfer",
"transport",
"travel",
"treat",
"treatment",
"treaty",
"tree",
"trend",
"trial",
"trip",
"troop",
"trouble",
"true",
"trust",
"truth",
"try",
"turn",
"twice",
"type",
"typical",
"unable",
"under",
"understand",
"understanding",
"undertake",
"unemployment",
"unfortunately",
"union",
"unit",
"united",
"university",
"unless",
"unlikely",
"until",
"up",
"upon",
"upper",
"urban",
"us",
"use",
"used",
"useful",
"user",
"usual",
"usually",
"value",
"variation",
"variety",
"various",
"vary",
"vast",
"vehicle",
"version",
"very",
"via",
"victim",
"victory",
"video",
"view",
"village",
"violence",
"vision",
"visit",
"visitor",
"vital",
"voice",
"volume",
"vote",
"wage",
"wait",
"walk",
"wall",
"want",
"war",
"warm",
"warn",
"wash",
"watch",
"water",
"wave",
"way",
"we",
"weak",
"weapon",
"wear",
"weather",
"week",
"weekend",
"weight",
"welcome",
"welfare",
"well",
"west",
"western",
"what",
"whatever",
"when",
"where",
"whereas",
"whether",
"which",
"while",
"whilst",
"white",
"who",
"whole",
"whom",
"whose",
"why",
"wide",
"widely",
"wife",
"wild",
"will",
"win",
"wind",
"window",
"wine",
"wing",
"winner",
"winter",
"wish",
"with",
"withdraw",
"within",
"without",
"woman",
"wonder",
"wonderful",
"wood",
"word",
"work",
"worker",
"working",
"works",
"world",
"worry",
"worth",
"would",
"write",
"writer",
"writing",
"wrong",
"yard",
"yeah",
"year",
"yes",
"yesterday",
"yet",
"you",
"young",
"your",
"yourself",
"youth"};
