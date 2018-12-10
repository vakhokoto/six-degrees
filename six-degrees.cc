#include <vector>
#include <list>
#include <set>
#include <string>
#include <iostream>
#include <iomanip>
#include <queue>
#include <map>
#include "imdb.h"
#include "path.h"
using namespace std;

const string IS_SOURCE = "source";

/**
 * Using the specified prompt, requests that the user supply
 * the name of an actor or actress.  The code returns
 * once the user has supplied a name for which some record within
 * the referenced imdb existsif (or if the user just hits return,
 * which is a signal that the empty string should just be returned.)
 *
 * @param prompt the text that should be used for the meaningful
 *               part of the user prompt.
 * @param db a reference to the imdb which can be used to confirm
 *           that a user's response is a legitimate one.
 * @return the name of the user-supplied actor or actress, or the
 *         empty string.
 */

static string promptForActor(const string& prompt, const imdb& db)
{
  string response;
  while (true) {
    cout << prompt << " [or <enter> to quit]: ";
    getline(cin, response);
    if (response == "") return "";
    vector<film> credits;
    if (db.getCredits(response, credits)) return response;
    cout << "We couldn't find \"" << response << "\" in the movie database. "
	 << "Please try again." << endl;
  }
}

bool pathCanBeFound(string &source, string &target, imdb &md){
  queue <pair <int, string> > q;
  q.push(make_pair(0, source));
  map <string, string> ansPath;
  map <string, film> filmName;
  set <film> f;
  ansPath[source] = IS_SOURCE;
  while (!q.empty()){
    string cur = q.front().second;
    int cur_l = q.front().first;
    if (q.front().first == 6){
      break;
    }
    q.pop();
    vector <film> films;
    md.getCredits(cur, films);
    bool ind = false;
    for (int i=0; i<films.size(); ++i){
      vector <string> actors;
      if (f.find(films[i]) != f.end()){
        continue;
      }
      f.insert(films[i]);
      md.getCast(films[i], actors);
      for (int j=0; j<actors.size(); ++j){
        string actor = actors[j];
        if (ansPath.find(actor) == ansPath.end()){
          ansPath[actor] = cur;
          filmName[actor] = films[i];
          q.push(make_pair(cur_l + 1, actor));
          if (actor == target){
            ind = true;
            break;
          }
        }
      }
      if (ind){
        break;
      }
    }
    if (ind){
      break;
    }
  }
  if (ansPath[target] == ""){
    return false;
  }
  path finalPath(source);
  vector <pair <string, film> > ans;
  string cur = target;
  while(cur != source){
    ans.push_back(make_pair(cur, filmName[cur]));
    cur = ansPath[cur];
  }
  for (int i=ans.size() - 1; i>-1; --i){
    finalPath.addConnection(ans[i].second, ans[i].first);
  }
  cout <<finalPath;
  return true;
}

/**
 * Serves as the main entry point for the six-degrees executable.
 * There are no parameters to speak of.
 *
 * @param argc the number of tokens passed to the command line to
 *             invoke this executable.  It's completely ignored
 *             here, because we don't expect any arguments.
 * @param argv the C strings making up the full command line.
 *             We expect argv[0] to be logically equivalent to
 *             "six-degrees" (or whatever absolute path was used to
 *             invoke the program), but otherwise these are ignored
 *             as well.
 * @return 0 if the program ends normally, and undefined otherwise.
 */

int main(int argc, const char *argv[])
{
  imdb db(determinePathToData(argv[1])); // inlined in imdb-utils.h
  if (!db.good()) {
    cout << "Failed to properly initialize the imdb database." << endl;
    cout << "Please check to make sure the source files exist and that you have permission to read them." << endl;
    return 1;
  }
  
  while (true) {
    string source = promptForActor("Actor or actress", db);
    if (source == "") break;
    string target = promptForActor("Another actor or actress", db);
    if (target == "") break;
    if (source == target) {
      cout << "Good one.  This is only interesting if you specify two different people." << endl;
    } else {
      if (!pathCanBeFound(source, target, db)){
        cout << endl << "No path between those two people could be found." << endl << endl;
      }
    }
  }
  
  cout << "Thanks for playing!" << endl;
  return 0;
}

