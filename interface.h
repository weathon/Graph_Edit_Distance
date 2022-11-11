#include "Application.h"
#include "Graph.h"
#include "Utility.h"
#include "Timer.h"
#include "popl.hpp"
#include "Application.cpp"

using namespace std;
using namespace popl;

void print_usage()
{
	printf("Usage: ./ged -h -d database_file -q query_file -m running_mode -p search_paradigm -l lower_bound_method -t ged_threshold\n");
	printf("**** Note that for GED verification, if the returned value is not -1, then it is only an upper bound of (and may be larger than) the exact GED\n\n");
}

ui label2int(const char *str, map<string, ui> &M)
{
	if (M.find(string(str)) == M.end())
		M[string(str)] = M.size();
	return M[string(str)];
}

ui load_db(const char *file_name, vector<Graph *> &graphs, map<string, ui> &vM, map<string, ui> &eM, int N = -1)
{
	FILE *fin = Utility::open_file(file_name, "r");

	const ui MAX_LINE = 1024;
	char line[MAX_LINE];
	if (fgets(line, MAX_LINE, fin) == NULL)
	{
		fclose(fin);
		return 0;
	}

	ui max_n = 0;
	while (line[0] == 't')
	{
		char buf[128], buf1[128];
		sscanf(line + 2, "%s%s", buf1, buf);
		string id(buf);
		line[0] = 'x';

		vector<pair<int, ui>> vertices;
		vector<pair<pair<int, int>, ui>> edges;
		while (fgets(line, MAX_LINE, fin) != NULL && line[0] != 't')
		{
			if (line[0] == 'v')
			{
				int a;
				sscanf(line + 2, "%d%s", &a, buf);
				// buf[0] = '1';
				vertices.pb(mp(a, label2int(buf, vM)));
			}
			else if (line[0] == 'e')
			{
				int a, b;
				sscanf(line + 2, "%d%d%s", &a, &b, buf);
				edges.pb(mp(mp(a, b), label2int(buf, eM)));
				edges.pb(mp(mp(b, a), label2int(buf, eM)));
			}
			else
				printf("!!! Unrecongnized first letter in a line when loading DB!\n");
			line[0] = 'x';
		}

		sort(vertices.begin(), vertices.end());
		for (ui i = 0; i < vertices.size(); i++)
			assert(vertices[i].first == i);
		if (vertices.size() > max_n)
			max_n = vertices.size();

		sort(edges.begin(), edges.end());
		for (ui i = 0; i < edges.size(); i++)
		{
			assert(edges[i].first.first >= 0 && edges[i].first.first < vertices.size());
			assert(edges[i].first.second >= 0 && edges[i].first.second < vertices.size());
			if (i > 0)
				assert(edges[i].first != edges[i - 1].first);
			assert(edges[i].second < eM.size());
		}

		graphs.pb(new Graph(id, vertices, edges));
	}

	fclose(fin);
	return max_n;
}

void generate_queries(const vector<Graph *> &db, vector<ui> &queries, ui q_n)
{
	assert(!db.empty());
	srand(time(NULL));
	for (ui i = 0; i < q_n; i++)
		queries.pb(rand() % db.size());
}

void write_queries(const char *file_name, const vector<Graph *> &db, const vector<ui> &queries, const map<string, ui> &vM, const map<string, ui> &eM, bool bss)
{
	vector<string> vlabels(vM.size());
	vector<string> elabels(eM.size());

	for (pair<string, ui> p : vM)
		vlabels[p.second] = p.first;
	for (pair<string, ui> p : eM)
		elabels[p.second] = p.first;

	FILE *fout = Utility::open_file(file_name, "w");

	for (ui i = 0; i < queries.size(); i++)
		db[queries[i]]->write_graph(fout, vlabels, elabels, bss);

	fclose(fout);
}

vector<Graph *> db;
ui verify_upper_bound;
int *vlabel_cnt;
int *elabel_cnt;

int *degree_q;
int *degree_g;
int *tmp;
string mode, paradigm, lower_bound;

bool verification(int id1, int id2, int vub, bool checkLB=true) // If the real dis is lower than vub
{
	verify_upper_bound = vub;
	if (checkLB)
	{
		ui lb = db[id1]->ged_lower_bound_filter(db[id2], vub, vlabel_cnt, elabel_cnt, degree_q, degree_g, tmp);
		if (lb > verify_upper_bound)
			return false;
	}
	Application *app = new Application(verify_upper_bound, "BMao");
	app->init(db[id1], db[id2]);
	int res = INF;
	res = app->AStar();
	if (res <= verify_upper_bound)
		return true;
	return false;
}
// cxyueduzhelidaimazhsodaolkunxiswuwufajlxlaiyuedukkzjzangkkzachidxzlilbdeyucecuileaaaakunkkkkun
/// ublb could be reuse witjin calls and begin and stack kunkoukekunkouxhaojiganyachi
bool verLessOrEqu(int id1, int id2, int vub, int lb, int &lbreturn, int &cache) // If the real dis is lower than vub
{

	// if(cache != -1) return cache; //jiaosuankunchaojihoajisoaunkunyunkunyunsuannashouyachi kundebuxingwhyhhgehuiyouernyo wotamadezaixiangshetaikunle kunchoajiyuntt
	// if(cache != -1)
	// {
	// 	cout<<"cache:"<<cache<<endl;
	// 	return (cache<=verify_upper_bound);
	// }
	cache = -1;
	if (vub < 0)
		return false;
	verify_upper_bound = vub;
	if (lb == -1)
	{
		lb = db[id1]->ged_lower_bound_filter(db[id2], INF, vlabel_cnt, elabel_cnt, degree_q, degree_g, tmp);
		lbreturn = lb;
	}
	// cout<<vub<<endl;// zhiqianmeiyou print vubgaiucolekoukehsouzhiyemeizaireturnzhiqian
	if(vub<0)
	{
		cout<<"🔥🔥🔥🔥🔥🔥🔥🔥🔥🔥🔥"<<endl;
	}
	if (lb > verify_upper_bound)
		return false;
	Application *app = new Application(verify_upper_bound, "BMao");
	app->init(db[id1], db[id2]);
	int res = INF;
	res = app->AStar();

	delete app;

	// cout << "res,vub=" << res << " " << verify_upper_bound << endl;
	if (res <= vub)
	{
		return true;
	}
	cache = -1;
	return false;
	// cout<<"tmp:+++++++++++++++++++++++++++++"<<*tmp;
	// cout<<vub<<endl;
	// verify_upper_bound = vub;
	// ui lb = db[id1]->ged_lower_bound_filter(db[id2], vub, vlabel_cnt, elabel_cnt, degree_q, degree_g, tmp);
	// cout<<"lb:"<<lb<<endl;
	// if (lb > verify_upper_bound)
	// {cout<<"false"<<endl;	return false;} //lb=7, vub is an negative number, then why it did not enter here??????? kunyunienskun nasnhoujiusuanmeiyou
	// Application *app = new Application(verify_upper_bound, "BMao");
	// app->init(db[id1], db[id2]);
	// int res = INF;
	// res = app->AStar();
	// cout<<"res:"<<res<<endl;
	// if (res <= verify_upper_bound)
	// 	delete app;
	// 	return true;
	// delete app;
	// return false;
}

/*bool verMoreOrEqu(int id1, int id2, int vub) //If the real dis is lower than vub
{
	if(vub<0)
		return true;
	verify_upper_bound = vub;
//	ui lb = db[id1]->ged_lower_bound_filter(db[id2], vub, vlabel_cnt, elabel_cnt, degree_q, degree_g, tmp);
//	if (lb > verify_upper_bound)
//		return false;
	Application *app = new Application(verify_upper_bound, "BMao");
	app->init(db[id1], db[id2]);
	int res = INF;
	res = app->AStar();
	if (res >= verify_upper_bound)
		delete app;
		return true;
	delete app;
	return false;
}*/
int query(int id1, int id2)
{

	// ui lb = db[id1]->ged_lower_bound_filter(db[id2], verify_upper_bound, vlabel_cnt, elabel_cnt, degree_q, degree_g, tmp);
	// cout<<"lb = "<<lb<<endl;
	// if (lb > verify_upper_bound)
	// 	return -1;
	// need the vub not the lb!!!!!!!! The result before is not useable! chaojikun hcaojikun yunnashoueixn quanbuhconglai
	// WHY!!?!?!?jiaotong

	// ++candidates_cnt;
	Timer t1;

	Application *app = new Application(verify_upper_bound, "BMao");
	app->init(db[id1], db[id2]);
	int res = INF;

	res = app->AStar();

	// search_space += app->get_search_space();
	if (res <= verify_upper_bound)
	{
		// ++results_cnt;
	}
	else
		res = -1;

	// printf("%d", res);

	delete app;
	return res;
}

void init()
{
	string path = "../dataForReal/29/graphs.txt";
	int threshold = -1;
	bool print_ged = false;

	string database = path;

	map<string, ui> vM, eM;
	ui max_db_n = load_db(database.c_str(), db, vM, eM);
	// printf("%ld\n", db.size());
	if (threshold < 0)
		verify_upper_bound = INF;
	else
		verify_upper_bound = (ui)threshold;

	long long search_space = 0;
	long long results_cnt = 0, candidates_cnt = 0;

	ui pre = 1000;

	vlabel_cnt = new int[vM.size()];
	elabel_cnt = new int[eM.size()];
	memset(vlabel_cnt, 0, sizeof(int) * vM.size());
	memset(elabel_cnt, 0, sizeof(int) * eM.size());

	degree_q = new int[max_db_n];
	degree_g = new int[max_db_n];
	tmp = new int[max_db_n];

	Timer t;

	long long time1 = 0, cnt1 = 0, ss1 = 0;
	long long time2 = 0, cnt2 = 0, ss2 = 0;

	ui min_ged = 1000000000, max_ged = 0;
}

void clean_up()
{
	delete[] vlabel_cnt;
	vlabel_cnt = NULL;
	delete[] elabel_cnt;
	elabel_cnt = NULL;
	delete[] degree_q;
	degree_q = NULL;
	delete[] degree_g;
	degree_g = NULL;
	delete[] tmp;
	tmp = NULL;

	for (ui i = 0; i < db.size(); i++)
	{
		delete db[i];
		db[i] = nullptr;
	}
}

int _main()
{
	init();
	cout << query(0, 1) << endl;
	cout << query(0, 3) << endl;
	cout << query(0, 5) << endl;
	clean_up();
	return 0;
}
