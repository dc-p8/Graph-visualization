//g++ main.cpp -lsfml-graphics -lsfml-window -lsfml-system -O3
#include <SFML/Graphics.hpp>
#include <vector>
#include <boost/graph/adjacency_list.hpp>
#include <iostream>
#include <random>
#include <cassert>
#include <algorithm>
#include <set>
#include <string> 
#include <ctime>

#define MODE_DEFAULT 0
#define MODE_ADD_VERTEX 1
#define MODE_ADD_EDGE 2
#define MODE_DELETE_VERTEX 3
#define MODE_DELETE_EDGE 4
#define MODE_DRAG 5

using namespace std;
using namespace boost;
using namespace sf;

class node{
private:
    static int number;
public:
    float x;
    float y;
    Vector2f repulsive;
    Vector2f attractive;
    float mass; // unused
    Color color;
    int id; // unused

    node(float x, float y){
        this->x = x;
        this->y = y;
        this->repulsive = Vector2f();
        this->attractive = Vector2f();
        this->mass = 1.0;
        int ncolor = 3;
        int r = 255 * (float)(random() % (ncolor + 1)) / (float)ncolor;
        int g = 255 * (float)(random() % (ncolor + 1)) / (float)ncolor;
        int b = 255 * (float)(random() % (ncolor + 1)) / (float)ncolor;
        this->color = Color(r, g, b);
        this->id = number++;
    }
    node() : node((((float)random() / (float)RAND_MAX)) - 0.5, (((float)random() / (float)RAND_MAX)) - 0.5){}
    ~node(){}
};

typedef adjacency_list<vecS, vecS, undirectedS, node, no_property> Graph;
typedef boost::graph_traits<Graph>::edge_descriptor edge_descriptor;
typedef boost::graph_traits<Graph>::vertex_descriptor vertex_descriptor;

int node::number = 0;

RenderWindow *window ;
Graph g;
bool rnd = false;
int mode = MODE_DEFAULT;
bool mode_return_ADD_node = false;

vertex_descriptor closest_mouse_vertex = -1;
vertex_descriptor selected_vertex = -1;
Vector2f mousecoords = {0, 0};
float default_speed = 0.1;
float speed = default_speed;
float attractive_power = 0.1;
float repulsive_power =  0.01;
bool re_center = true;
bool stress = false;
float re_center_power = 0.25;
//float maximum_distance_color = sqrt(255 * 255 * 3);

// Fixed in main
CircleShape shape;

// Fixed in main_loop
Vector2f worldsize;
float center_mass_x;
float center_mass_y;
float maximum_distance;
Vector2u window_size;
float minx;
float maxx;
float miny;
float maxy;
float max_speed;
//Font font

void randomize()
{
	speed = default_speed;
	for(auto it = vertices(g); it.first != it.second; ++it.first)
    {
        auto descriptor = *(it).first;
        auto node = &g[descriptor];
        node->x = (((float)random() / (float)RAND_MAX)) - 0.5;
        node->y = (((float)random() / (float)RAND_MAX)) - 0.5;
    }
}

void make_circle(Graph &g, int nb_vertex)
{
	vertex_descriptor last = -1;
    vertex_descriptor first = -1;
    for(int i = 0; i < nb_vertex; ++i)
    {
        float x = /*0.5 + */((((float)random() / (float)RAND_MAX)) - 0.5);
        float y = /*0.5 + */((((float)random() / (float)RAND_MAX)) - 0.5);
        auto v = add_vertex( node(x, y), g);
        if(i == 0)
        	first = v;
        if(last != -1)
        	add_edge(v, last, g);
        last = v;
    }
    if(first != -1 && last != -1)
    {
    	add_edge(first, last, g);
    }
}



void random_graph(Graph& g, int nb_vertex) {

    for(int i = 0; i < nb_vertex; ++i)
    {
        float x = /*0.5 + */((((float)random() / (float)RAND_MAX)) - 0.5);
        float y = /*0.5 + */((((float)random() / (float)RAND_MAX)) - 0.5);
        auto v = add_vertex(g);
        vertex_descriptor v2 = v;
        if(i > 1)
        {
	        int rand = random() % i;
	         int j = 0;
	        for(auto pair_it = vertices(g); pair_it.first != pair_it.second, j < rand; ++pair_it.first)
	        {
	        	v2 = *(pair_it).first;
	        	j++;
	        }
	        if(v2 != v)
	        	add_edge(v2, v, g);
    	}
    }

}

void find_closest()
{
	float distance_min = 10000;
	for(auto it = vertices(g); it.first != it.second; ++it.first)
    {
        auto descriptor = *(it).first;
        auto node = &g[descriptor];
        float dx = node->x - mousecoords.x;
        float dy = node->y - mousecoords.y;
        float distance = dx*dx + dy*dy;
        if(distance < distance_min)
        {
        	distance_min = distance;
        	closest_mouse_vertex = descriptor;
        }
    }
}

void inputs()
{
	Event event;
	stress = false;
	if(Keyboard::isKeyPressed (Keyboard::S))
	{
		stress = true;
	}
	if(Keyboard::isKeyPressed (Keyboard::Add))
	{
		if(speed == 0)
			speed = default_speed;
		else
			speed += 0.01 * default_speed;

		printf("multiplier : %f\n", speed);
	}
	if(Keyboard::isKeyPressed (Keyboard::Subtract))
	{
		speed -= 0.01 * default_speed;
		if(speed < 0)
			speed = 0;
		printf("multiplier : %f\n", speed);
	}

	// Compute mouse coordinates un world space
	Vector2i mousePos = Mouse::getPosition(*window);
    float w_X = (float)mousePos.x / (float)window_size.x;
	float w_Y = (float)mousePos.y / (float)window_size.y;
	w_X = ((w_X * (worldsize.x)) + minx) * 1.1;
	w_Y = ((w_Y * (worldsize.y)) + miny) * 1.1;
	if(w_X < minx)
		w_X = minx;
	if(w_X > maxx)
		w_X = maxx;
	if(w_Y < miny)
		w_Y = miny;
	if(w_Y > maxy)
		w_Y = maxy;
	mousecoords.x = w_X;
	mousecoords.y = w_Y;

    while (window->pollEvent(event))
    {
        // Close window: exit
        if (event.type == Event::Closed)
            window->close();

        if(event.type == Event::KeyPressed)
        {
        	if(event.key.code == Keyboard::Q)
			{
				window->close();
			}
			if(event.key.code == Keyboard::R)
			{
				randomize();
			}
			if(event.key.code == Keyboard::C)
			{
				re_center = !re_center;
			}
        	else if(event.key.code == Keyboard::Escape)
        	{
        		mode = MODE_DEFAULT;
        		closest_mouse_vertex = -1;
		 		selected_vertex = -1;
        	}
        	else if(event.key.code == Keyboard::V)
        	{
        		if(event.key.shift)
        			mode = MODE_DELETE_VERTEX;
        		else
        			mode = MODE_ADD_VERTEX;
        	}
        	else if(event.key.code == Keyboard::E)
        	{
        		if(event.key.shift)
        			mode = MODE_DELETE_EDGE;
        		else
        			mode = MODE_ADD_EDGE;
        		selected_vertex = -1;
        	}
        	else if(event.key.code == Keyboard::M)
        	{
        		mode = MODE_DRAG;
        		selected_vertex = -1;
        	}

        }

        if( (event.type == Event::MouseButtonPressed) && (event.mouseButton.button == Mouse::Left) )
	 	{
	 		if(mode == MODE_DELETE_VERTEX && closest_mouse_vertex != -1)
	 		{
	 			clear_vertex(closest_mouse_vertex, g);
	 			remove_vertex(closest_mouse_vertex, g);
	 			closest_mouse_vertex = -1;
	 		}
	 		else if(mode == MODE_ADD_EDGE && closest_mouse_vertex != -1)
	 		{
	 			if(selected_vertex == -1)
	 			{
	 				selected_vertex = closest_mouse_vertex;
	 			}
	 			else if(selected_vertex != closest_mouse_vertex && !edge(selected_vertex, closest_mouse_vertex, g).second)
	 			{
	 				// Color col1 = g[selected_vertex].color;
		 			// Color col2 = g[closest_mouse_vertex].color;
		 			add_edge(selected_vertex, closest_mouse_vertex, g);
		 			closest_mouse_vertex = -1;
		 			selected_vertex = -1;
		 			if(mode_return_ADD_node)
		 			{
		 				mode_return_ADD_node = false;
		 				mode = MODE_ADD_VERTEX;
		 			}
		 			
		 			// int dr = col2.r - col1.r;
		 			// int dg = col2.g - col1.g;
		 			// int db = col2.b - col1.b;

		 			// float distance_color = sqrt( (dr * dr) + (dg * dg) + (db * db) ) / maximum_distance_color;
		 			// printf("%d %d %d | %d %d %d ---->>> %f\n", col1.r, col1.g, col1.b, col2.r, col2.g, col2.b, distance_color);
		 		}
	 		}
	 		else if(mode == MODE_DELETE_EDGE && closest_mouse_vertex != -1)
	 		{
	 			if(selected_vertex == -1)
	 			{
	 				selected_vertex = closest_mouse_vertex;
	 			}
	 			else if(selected_vertex != closest_mouse_vertex && edge(selected_vertex, closest_mouse_vertex, g).second)
	 			{
		 			remove_edge(selected_vertex, closest_mouse_vertex, g);
		 			closest_mouse_vertex = -1;
		 			selected_vertex = -1;
		 		}
	 		}
	 		else if(mode == MODE_ADD_VERTEX)
	 		{
	 			selected_vertex = add_vertex(node(mousecoords.x, mousecoords.y), g);
	 			mode = MODE_ADD_EDGE;
	 			mode_return_ADD_node = true;
	 		}
	 		else if(mode == MODE_DRAG && closest_mouse_vertex != -1)
	 		{
	 			selected_vertex = closest_mouse_vertex;
	 		}
		}
		if( (event.type == Event::MouseButtonReleased) && (event.mouseButton.button == Mouse::Left) )
	 	{
	 		if(mode == MODE_DRAG && closest_mouse_vertex != -1)
	 		{
		 		selected_vertex = -1;
	 		}
		}
    }
}

void simulation()
{
	// Repulsive
	for(auto it = vertices(g); it.first != it.second; ++it.first)
    {
        auto descriptor1 = *(it).first;
        auto cnode1 = &g[descriptor1];
        cnode1->repulsive.x = 0.0;
        cnode1->repulsive.y = 0.0;
        for(auto it_a = vertices(g); it_a.first != it_a.second; ++it_a.first)
        {
            auto descriptor2 = *(it_a).first;
            if(descriptor2 != descriptor1)
            {
                auto cnode2 = &g[descriptor2];
                float dx = cnode1->x - cnode2->x;
                float dy = cnode1->y - cnode2->y;
                float distance = sqrt(dx * dx + dy * dy);

                if(distance != 0.0)
                {
                    float normalized_dostance_x = dx / distance;
                    float normalized_dostance_y = dy / distance;
                    cnode1->repulsive.x += (normalized_dostance_x / distance);
                    cnode1->repulsive.y += (normalized_dostance_y / distance);
                }
            }
        }
    }

    // Attractive
    for(auto it = vertices(g); it.first != it.second; ++it.first)
    {
        auto descriptor1 = *(it).first;
        auto cnode1 = &g[descriptor1];
        cnode1->attractive.x = 0.0;
        cnode1->attractive.y = 0.0;

        if(re_center)
        {
            float dx = cnode1->x - center_mass_x;
            float dy = cnode1->y - center_mass_y;
            float distance = sqrt(dx * dx + dy * dy);
            float ratio = (distance / maximum_distance);
            //printf("%f\n", ratio);
            if(distance != 0.0)
            {
                float invert_normalized_dostance_x = dx / distance * -1.0;
                float invert_normalized_dostance_y = dy / distance * -1.0;
                cnode1->attractive.x += invert_normalized_dostance_x * (distance * distance) * ratio * re_center_power;
                cnode1->attractive.y += invert_normalized_dostance_y * (distance * distance) * ratio * re_center_power;
            }
        }

        for(auto it_a = adjacent_vertices(descriptor1, g); it_a.first != it_a.second; ++it_a.first)
        {
            auto descriptor2 = *(it_a).first;
            auto cnode2 = &g[descriptor2];
            float dx = cnode1->x - cnode2->x;
            float dy = cnode1->y - cnode2->y;
            float distance = sqrt(dx * dx + dy * dy);

            if(distance != 0.0)
            {
                float invert_normalized_dostance_x = dx / distance * -1.0;
                float invert_normalized_dostance_y = dy / distance * -1.0;
                cnode1->attractive.x += (invert_normalized_dostance_x * (distance * distance));
                cnode1->attractive.y += (invert_normalized_dostance_y * (distance * distance));
            }
        }
    }

    // Applying forces
    for(auto it = vertices(g); it.first != it.second; ++it.first)
    {
        auto descriptor = *(it).first;
        if(mode == MODE_DRAG && descriptor == selected_vertex)
        	continue;
        auto node = &g[descriptor];

        node->attractive.x *= attractive_power;
        node->attractive.y *= attractive_power;
        node->repulsive.x *= repulsive_power;
        node->repulsive.y *= repulsive_power;

        // Limit the forces
        float ax = (node->attractive.x + node->repulsive.x) * speed;
        float ay = (node->attractive.y + node->repulsive.y) * speed;

        if(abs(ax) > 0.01 * worldsize.x)
    	{
    		if(ax < 0)
    			ax = -1.0 * (0.01 * worldsize.x);
    		else
    			ax = 0.01 * worldsize.x;
    	}

    	if(abs(ay) > 0.01 * worldsize.y)
    	{
    		if(ay < 0)
    			ay = -1.0 * (0.01 * worldsize.y);
    		else
    			ay = 0.01 * worldsize.y;
    	}

        node->x += ax;
        node->y += ay;

        if(stress)
        {
        	node->x += (((float)random() / (float)RAND_MAX) * 0.01 * worldsize.x);
        	node->y += (((float)random() / (float)RAND_MAX) * 0.01 * worldsize.y);
        }
    }
}

void draw()
{
 // Clear screen
    window->clear(Color(127, 127, 127));

    // Draw center of mass
    Color col {0, 0, 0, 100};
    if(re_center)
    	col = {255, 255, 255, 100};

    
    shape.setScale(0.05  * worldsize.x, 0.05  * worldsize.y);
    shape.setPosition(center_mass_x , center_mass_y );
    shape.setFillColor(col);
    shape.setOutlineThickness(0);
	shape.setOutlineColor({0, 0, 0, 0});
    window->draw(shape);

    // Draw all edges
    for(auto it = edges(g); it.first != it.second; ++it.first)
    {
        auto descriptor = *(it).first;
        auto cnode1 = g[source(descriptor, g)];
        auto cnode2 = g[target(descriptor, g)];
        Vertex line[] =
        {
            Vertex(Vector2f(cnode1.x , cnode1.y ), cnode1.color),
            Vertex(Vector2f(cnode2.x , cnode2.y ), cnode2.color),
        };
        window->draw(line, 2, Lines);
    }

    // Draw all vertex
    shape.setScale(0.005  * worldsize.x, 0.005  * worldsize.y);
    for(auto it = vertices(g); it.first != it.second; ++it.first)
    {
        auto descriptor = *(it).first;
        auto cnode = &g[descriptor];
        shape.setPosition(cnode->x , cnode->y );
        shape.setFillColor(cnode->color);
        window->draw(shape);

        /*
        Text text;
        text.setFont(font);
        text.setString(to_string(cnode->id));
        text.setPosition(cnode->x * window_size.x, cnode->y * window_size.y);
        text.setCharacterSize(0.05 * window_size.x / 2.0);
        text.setOrigin(text.getLocalBounds().width / 2.0, text.getLocalBounds().height / 2.0);
        //text.setScale(0.005 * window_size.x, 0.005 * window_size.y);
        text.setFillColor(Color(255, 255, 255));
        window->draw(text);
        */
    }

    // Draw mouse position
    Color color;
	if(mode == MODE_ADD_VERTEX)
		color = Color(0, 255, 0, 255);
	else
		color = Color(255, 0, 0, 100);
	
    shape.setScale(0.01  * worldsize.x, 0.01  * worldsize.y);
    shape.setPosition(mousecoords.x, mousecoords.y);
    shape.setFillColor(color);
    window->draw(shape);
    
    // Draw selected vertex and the closest vertex to the mose
    if(closest_mouse_vertex != -1 && mode != MODE_ADD_VERTEX )
    {
	    Color color_closest = Color(255, 255, 255, 255);
	    Color color_selected = Color(255, 255, 255, 255);

	    if(mode == MODE_DELETE_VERTEX || mode == MODE_DELETE_EDGE)
	    {
	    	color_closest = Color(255, 0, 0, 255);
	    	color_selected = Color(255, 150, 150, 255);
	    }

	    if(mode == MODE_ADD_VERTEX || mode == MODE_ADD_EDGE)
	    {
	    	color_closest = Color(0, 255, 0, 255);
	    	color_selected = Color(150, 255, 150, 255);
	    }

    	auto node = &g[closest_mouse_vertex];
	    shape.setScale(0.01  * worldsize.x, 0.01  * worldsize.y);
	    shape.setPosition(node->x, node->y);
	    shape.setFillColor(Color(0, 0, 0, 0));
	    shape.setOutlineThickness(0.001 * ((window_size.x + window_size.y) / 2.0));
		shape.setOutlineColor(color_closest);
	    window->draw(shape);

	    if(selected_vertex != -1)
	    {
	    	auto node = &g[selected_vertex];
	        shape.setPosition(node->x, node->y);
			shape.setOutlineColor(color_selected);
	        window->draw(shape);
	    }
	}
    

    window->display();
}

void compute_bounds()
{
	window_size = window->getSize();
	minx = 0;
	maxx = 0;
	miny = 0;
	maxy = 0;
    float allx = 0;
    float ally = 0;
    int i = 0;

    // get bounds, center, center of mass
    for(auto it = vertices(g); it.first != it.second; ++it.first)
    {
        auto descriptor = *(it).first;
        auto cnode = &g[descriptor];
        allx += cnode->x;
        ally += cnode->y;
        if(cnode->x < minx)
            minx = cnode->x;
        if(cnode->x > maxx)
            maxx = cnode->x;
        if(cnode->y < miny)
            miny = cnode->y;
        if(cnode->y > maxy)
            maxy = cnode->y;
        i++;
    }
    float center_x = minx + (maxx - minx) / 2.0;
    float center_y = miny + (maxy - miny) / 2.0;
    center_mass_x = allx / (float)i;
    center_mass_y = ally / (float)i;


    // Center all vertex to make the view follow their move
    for(auto it = vertices(g); it.first != it.second; ++it.first)
    {
        auto descriptor = *(it).first;
        auto cnode = &g[descriptor];
        cnode->x -= center_x;
        cnode->y -= center_y;
    }

    // Fix the view to encompass all vertex + space
    auto v = View();
    worldsize = {abs(minx) + abs(maxx), abs(miny) + abs(maxy)};
    v.setCenter(0, 0);
    v.setSize(worldsize.x * 1.1, worldsize.y * 1.1);
    window->setView(v);
}

void main_loop()
{
	while (window->isOpen())
    {
    	compute_bounds();

    	// Compute the maximum distance (used to scale center repultion power)
	    float dx = maxx - minx;
	    float dy = maxy - miny;
	    maximum_distance = sqrt(dx * dx + dy * dy);
	    max_speed = 0.01 * maximum_distance;

        inputs();
  
        if(mode != MODE_DEFAULT)
        	find_closest();

        draw();

        //getchar();
        simulation();

        

        // In drag and drop mode, move selected vertex
        if(mode == MODE_DRAG && selected_vertex != -1)
        {
        	 auto node = &g[selected_vertex];
        	 node->x = mousecoords.x;
        	 node->y = mousecoords.y;
        }
        
    }
}


int main(int argc, char **argv)
{
    //srand(time(NULL));
    // if (!font.loadFromFile("monofonto.ttf"))
    // {
    //     exit(1);
    // }
    
    int n = 200;
    if(argc > 1)
    {
    	n = stoi(argv[1]);
    }

    random_graph(g, n);
    //make_circle(g, n);

    while(true)
    {
    	int nb_alone = 0;
    	for(auto it = vertices(g); it.first != it.second; ++it.first)
        {
        	auto descriptor = *(it).first;
	        if (degree(descriptor, g) == 0)
	        {
	            remove_vertex(descriptor, g);
	            nb_alone ++;
	            break;
	        }
        }
        if(nb_alone == 0)
        	break;
    }


    ContextSettings settings;
    settings.antialiasingLevel = 8;
    RenderWindow tmp(VideoMode(800, 800), "SFML window", Style::Default,settings);
    window = &tmp;

    shape = CircleShape(1, 30);
    shape.setOrigin(1, 1);

    main_loop();
    // Start the game loop

    return EXIT_SUCCESS;
}