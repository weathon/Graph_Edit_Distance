#include "Application.h"
#include "Graph.h"
#include "Utility.h"
#include "Timer.h"
#include "popl.hpp"

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

ui load_db(const char *file_name, vector<Graph *> &graphs, map<string, ui> &vM, map<string, ui> &eM)
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

int *vlabel_cnt;
int *elabel_cnt;

int *degree_q;
int *degree_g;
int *tmp;
string mode, paradigm, lower_bound;

struct trio{
	int a,b,c;
};

std::ostream& operator<<(std::ostream& os, const trio& obj) {
	// by chatgpt
    os << obj.a<< " "<<obj.b<<" "<<obj.c;
    return os;
}

trio verify(int id1, int id2, ui verify_upper_bounds[3]) //return 1 if >= 
{
	ui min_verify_upper_bound = INF; //why i set it as -1
	ui max_verify_upper_bound = 0;
	for(int i=0; i<=3; i++)
	{
		min_verify_upper_bound = min(min_verify_upper_bound, verify_upper_bounds[i]);
		max_verify_upper_bound = max(max_verify_upper_bound, verify_upper_bounds[i]);
	}

	ui lb = db[id1]->ged_lower_bound_filter(db[id2], max_verify_upper_bound, vlabel_cnt, elabel_cnt, degree_q, degree_g, tmp);
	// cout<<"lb "<<lb<<endl;
	if (lb > max_verify_upper_bound)
		return trio{1, 1, 1};
	// this is not nessnery for ours because everything we have is already lower or equal to the lower bound
	// ++candidates_cnt;
	Timer t1;

	Application *app = new Application(min_verify_upper_bound,max_verify_upper_bound, "BMao");
	app->init(db[id1], db[id2]);
	int res = INF;

	res = app->AStar();

	// search_space += app->get_search_space();
	// if (res <= min_verify_upper_bound)
	// {
	// 	// ++results_cnt;
	// }
	// else
	// 	res = -1;

	// printf("%d", res);

	delete app;
	trio ret = trio{0,0,0};//forget about this
	// cout<<res<<endl; //naozibuqingchuduid e axkoukun meibaocun shijiangouchiaojiyunchaojiyunkunyunex
	if(res>=verify_upper_bounds[0])
		ret.a = 1;
	if(res>=verify_upper_bounds[1])
		ret.b = 1;
	if(res>=verify_upper_bounds[2])
		ret.c = 1;
	return ret;
}

void init()
{
	bool print_ged = false;

	string database = "./datasets/AIDS.txt";
	// string database = "../dataForReal/graphs.txt";

	map<string, ui> vM, eM;
	ui max_db_n = load_db(database.c_str(), db, vM, eM);


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

#
int main()
{
	init();
	ui v[]={5, 10, 26};
	// cout<<verify(0, 1, v)<<endl; //1
	// cout<<verify(0, 3, v)<<endl; //4 now it says 6
	// cout<<verify(0, 5, v)<<endl; //6
	// cout<<verify(0, 7, v)<<endl; 
	// cout<<verify(0, 8, v)<<endl; 
	// cout<<verify(0, 9, v)<<endl; 
	// cout<<verify(0, 10, v)<<endl; 
	// cout<<verify(0, 11, v)<<endl; 
	// cout<<verify(0, 12, v)<<endl; 
	// clean_up();
	// cout<<verify(0, 1, v)<<endl; //1
	// cout<<verify(0, 3, v)<<endl; //4
	// cout<<verify(0, 5, v)<<endl; //6
	// cout<<verify(0, 7, v)<<endl;  //6
	// cout<<verify(0, 51, v)<<endl; //11
	// cout<<verify(0, 50, v)<<endl;  //9
	// cout<<verify(0, 18, v)<<endl;  //8
	// cout<<verify(0, 14, v)<<endl;  //6
	// cout<<verify(0, 10, v)<<endl;  //6
	for(int i=0;i<=3000;i++)
	{
		cout<<verify(0, i, v)<<endl;  //6
	}
	return 0;
}
