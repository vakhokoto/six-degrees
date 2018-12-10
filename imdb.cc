using namespace std;
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/mman.h>
#include <fcntl.h>
#include <unistd.h>
#include <string.h>
#include <stdlib.h>
#include "imdb.h"

const char *const imdb::kActorFileName = "actordata";
const char *const imdb::kMovieFileName = "moviedata";

struct Node{
  const void * key;
  const void * file;
};

/*
  this function gets the (char *) pointer to info of film or actor
*/

char * getToInfo(char * ch, int &num){
  num = 1;
  while (*ch != 0){
    ch++;
    num++;
  }
  ch++;
  return ch;
}

/*
  get film info by number
*/

film imdb::getFilmInfo(int num) const {
  char * p = (char *)movieFile;
  p += num;
  film info;
  while (*p != 0){
    info.title += *p;
    p++;
  }
  int none;
  p = getToInfo(p, none);
  info.year = (int)*p + PLUS_FOR_MOVIE;
  return info;
}

/*
  reading out actor information
*/

void imdb::readActorInformation(char * start, vector <film>& films, int num) const{
  short cnt = *(short *) start;
  num += 2;
  start += num % 4 + 2;
  int * curPos = (int *) start;
  for (short i=0; i<cnt; ++i){
    int filmNumber = *curPos;
    film curFilm = getFilmInfo(filmNumber);
    films.push_back(curFilm);
    curPos++;
  }
}

/*
  get actor name by offset
*/

string imdb::getActorName(int offset) const {
  char * ch = (char *) actorFile;
  ch += offset;
  string ans = "";
  while (*ch != 0){
    ans += *ch;
    ch++;
  }
  return ans;
}

/*
  read film information from pointer
*/

void imdb::readFilmInformation(char* pointer, int length, vector <string> &players) const {
  if (length & 1){
    pointer++;
    length++;
  }
  short cnt = *(short*)pointer;
  length += 2;
  pointer += length % 4 + 2;
  for (short i=0; i<cnt; ++i){
    int offset = *(int *) pointer;
    pointer += sizeof(int);
    string now = getActorName(offset);
    players.push_back(now);
  }
}

/*
  actors comparison function
*/

int compareActors(const void* x, const void* y){
  Node * node = (Node *) x;
  char* str2 = (char *) node -> file + *(int *)y;
  char* str1 = *(char**) node -> key;
  return strcmp(str1, str2);
}

int compareFilms(const void* x, const void* y) {
  Node node = *(Node*)x;
  film key = *((film*)(node.key));
  char * file = (char*)node.file;
  int offset = *(int *)y;
  file += offset;
  film new_film;
  new_film.title = file;
  while (*file != 0){
    file++;
  }
  file++;
  new_film.year = 1900 + (int)(*(char*)file);
  if (key == new_film){
    return 0;
  } else if (key < new_film){
    return -1;
  }
  return 1;
}

imdb::imdb(const string& directory)
{
  const string actorFileName = directory + "/" + kActorFileName;
  const string movieFileName = directory + "/" + kMovieFileName;
  
  actorFile = acquireFileMap(actorFileName, actorInfo);
  movieFile = acquireFileMap(movieFileName, movieInfo);
}

bool imdb::good() const
{
  return !( (actorInfo.fd == -1) || 
	    (movieInfo.fd == -1) ); 
}

// you should be implementing these two methods right here... 
bool imdb::getCredits(const string& player, vector<film>& films) const {
  int size = *(int *)actorFile;
  int* first = (int *) actorFile + 1;
  Node node;
  node.key = &player;
  node.file = actorFile;
  int* ans = (int *) bsearch(&node, first, size, sizeof(int), compareActors);
  if (ans == NULL){
    return false;
  }
  char * ch = (char *)actorFile + *ans;
  int num = 0;
  ch = getToInfo(ch, num);
  if (num & 1){
    ch++;
    num++;
  }
  readActorInformation(ch, films, num);
  return true;
}
bool imdb::getCast(const film& movie, vector<string>& players) const {
  int nFilms = *(int *) movieFile;
  Node node;
  node.key = &movie;
  node.file = movieFile;
  void * start = (int*) movieFile + 1;
  int * ans = (int*)bsearch(&node, start, nFilms, sizeof(int), compareFilms);
  if (ans == NULL){
    return false;
  }
  char* pointer =(char*) movieFile + *(int *)ans;
  int length = 1;
  while (*pointer != 0){
    pointer++;
    length++;
  }
  pointer += 2;
  length++;
  readFilmInformation(pointer, length, players);
  return true;
}

imdb::~imdb()
{
  releaseFileMap(actorInfo);
  releaseFileMap(movieInfo);
}

// ignore everything below... it's all UNIXy stuff in place to make a file look like
// an array of bytes in RAM.. 
const void *imdb::acquireFileMap(const string& fileName, struct fileInfo& info)
{
  struct stat stats;
  stat(fileName.c_str(), &stats);
  info.fileSize = stats.st_size;
  info.fd = open(fileName.c_str(), O_RDONLY);
  return info.fileMap = mmap(0, info.fileSize, PROT_READ, MAP_SHARED, info.fd, 0);
}

void imdb::releaseFileMap(struct fileInfo& info)
{
  if (info.fileMap != NULL) munmap((char *) info.fileMap, info.fileSize);
  if (info.fd != -1) close(info.fd);
}