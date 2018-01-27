/*
*
*  @brief     Simple cluster analysis
*  @details   Unweighted pair-group average
*  @author    Matej Soroka
*  @date      12-13-2017
*
*/
#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
#include <string.h>
#include <ctype.h>
#include <math.h>
#include <limits.h>

///@defgroup array Array operations
///@defgroup cluster Cluster operations

#ifdef NDEBUG
#define debug(s)
#define dfmt(s, ...)
#define dint(i)
#define dfloat(f)
#else

/// prints debug string
#define debug(s) printf("- %s\n", s)

/// prints formated debug output (using similar like printf)
#define dfmt(s, ...) printf(" - "__FILE__":%u: "s"\n",__LINE__,__VA_ARGS__)

/// prints debug information about variable
#define dint(i) printf(" - " __FILE__ ":%u: " #i " = %d\n", __LINE__, i)

/// prints debug inforamtion about variable float type
#define dfloat(f) printf(" - " __FILE__ ":%u: " #f " = %g\n", __LINE__, f)

#endif

/// @struct obj_t
struct obj_t {

    int id;  ///< unique ID of object
    float x; ///< x coordinate of object
    float y; ///< y coordinate of object
};

/// @struct cluster_t
struct cluster_t {
    int size;           ///< number of objects in cluster
    int capacity;       ///< maximum number of objects in cluster
    struct obj_t *obj;  ///< array of objects in cluster
};

/**
*  Init of cluster. Allocate memory for capacity of object
*  pointer to NULL means zero capacity of array
*  @ingroup cluster
*  @param c pointer to cluster
*  @param cap capacity of cluster
*  @pre cluster can't point to NULL
*  @pre capacity must be greater than one
*/
void init_cluster(struct cluster_t *c, int cap)
{
    assert(c != NULL);
    assert(cap >= 0);

    c->size = 0;
    if (cap > 0)
    {
        if ((c->obj = malloc(cap * sizeof(struct obj_t))))
            c->capacity = cap;
        else
            fprintf(stderr, "Memory allocation was not succeed\n");
    }
    else
    {
        c->capacity = 0;
        c->obj = NULL;
    }
}

/**
*  Erase all objects in cluster and initialize
*  @ingroup cluster
*  cluster to empty cluster
*  @param c pointer to cluster
*/
void clear_cluster(struct cluster_t *c)
{
    free(c->obj);
    c->capacity = 0;
    c->size = 0;
    c->obj = NULL;
}

/// Chunk of cluster objects. Value recommended for reallocation.
const int CLUSTER_CHUNK = 10;

/**
*  Change capacity of cluster
*  @ingroup cluster
*  @param c pointer to cluster
*  @param new_cap new capacity
*  @pre cluster can't point to NULL
*  @pre cluster capacity must be greater than one
*  @pre new capacity must be greater than zero
*  @return pointer to resized structure
*/
struct cluster_t *resize_cluster(struct cluster_t *c, int new_cap)
{
    assert(c);
    assert(c->capacity >= 0);
    assert(new_cap >= 0);

    if (c->capacity >= new_cap)
        return c;

    size_t size = sizeof(struct obj_t) * new_cap;

    void *arr = realloc(c->obj, size);
    if (arr == NULL)
        return NULL;

    c->obj = (struct obj_t*)arr;
    c->capacity = new_cap;
    return c;
}

/**
*  @ingroup cluster
*  Appends object to cluster, in case of full capacity, resizes cluster
*  @param c pointer to cluster
*  @param obj object
*/
void append_cluster(struct cluster_t *c, struct obj_t obj)
{
    int cap = c->capacity;
    int size = c->size;
    while (size >= cap)
        cap += CLUSTER_CHUNK;

    resize_cluster(c, cap);
    c->obj[c->size++] = obj;
}

/*
 Seradi objekty ve shluku 'c' vzestupne podle jejich identifikacniho cisla.
 */
void sort_cluster(struct cluster_t *c);

/**
*  Appends objects from one cluster to another, in case can be resized.
*  After merge objects are sorted by id
*  @ingroup cluster
*  @param c1 pointer to cluster into which is appended
*  @param c2 pointer to cluster which objects are appended
*  @pre pointers c1 and c2 can't be NULL
*/
void merge_clusters(struct cluster_t *c1, struct cluster_t *c2)
{
    assert(c1 != NULL);
    assert(c2 != NULL);

    for (int i = 0; i < c2->size; i++)
        append_cluster(c1, c2->obj[i]);

    sort_cluster(c1);
}

/**********************************************************************/
/* Array operations */


/**
*  Appends objects from one cluster to another, in case can be resized.
*  After merge objects are sorted by id
*  @ingroup array
*  @param carr array of clusters
*  @param narr number of clusters in array
*  @param idx index in array which is removed
*  @pre index of object must be lower than number of clusters in array
*  @pre number of clusters in array must be greater than zero
*  @return size of cluster after remove
*/
int remove_cluster(struct cluster_t *carr, int narr, int idx)
{
    assert(idx < narr);
    assert(narr > 0);

    while (idx < narr) {

        clear_cluster(&carr[idx]);

        // if index is last in array, no need to merge
        if(idx != narr - 1)
            merge_clusters(&carr[idx], &carr[idx + 1]);

        idx++;

    }

    return narr - 1;

}

/**
*  Euclides distance between two objects
*  @ingroup cluster
*  @param o1 pointer to object
*  @param o2 pointer to object
*  @pre objects o1 and o2 can't point to NULL
*  @return euclides distance between two objects
*/
float obj_distance(struct obj_t *o1, struct obj_t *o2)
{
    assert(o1 != NULL);
    assert(o2 != NULL);

    float dist, a, b;

    a = o1->x - o2->x;
    b = o1->y - o2->y;

    a *= a;
    b *= b;

    dist = sqrtf(a + b);

    return dist;
}

/// Case value for choosing cluster distance method
int premium_case;

/**
*  Calculate distance between two clusters by selected method
*  @ingroup cluster
*  @param c1 pointer to cluster
*  @param c2 pointer to cluster
*  @pre clusters c1 and c2 can't point to NULL
*  @pre cluster size of cluster must be greater than zero
*  @return distance between two clusters
*/
float cluster_distance(struct cluster_t *c1, struct cluster_t *c2)
{
    assert(c1 != NULL);
    assert(c1->size > 0);
    assert(c2 != NULL);
    assert(c2->size > 0);

    float result;

    if(!premium_case)
    {

        float object_distance = 0;
        int object_count = 0;

        for (int i = 0; i < c1->size; i++)
        {
            for (int j = 0; j < c2->size; j++)
            {
                object_distance += obj_distance(&c1->obj[i], &c2->obj[j]);
                object_count++;
            }
        }
        result = object_distance / object_count;
    }

    if(premium_case == 1)
    {
        float distance = obj_distance(&c1->obj[0], &c2->obj[0]);

        for(int i = 0; i < c1->size; i++)
        {
            for(int j = 0; j < c2->size; j++)
            {
                float new_distance = obj_distance(&c1->obj[i], &c2->obj[j]);

                if(distance > new_distance)
                    distance = new_distance;
            }
        }
        result = distance;
    }

    if(premium_case == 2)
    {
        float distance = obj_distance(&c1->obj[0], &c2->obj[0]);

        for(int i = 0; i < c1->size; i++)
        {
            for(int j = 0; j < c2->size; j++)
            {
                float new_distance = obj_distance(&c1->obj[i], &c2->obj[j]);

                if(distance < new_distance)
                    distance = new_distance;
            }
        }
        result = distance;
    }

    return result;
}

/**
*  Searching for two closest clusters in array
*  and saves their indexes
*  @ingroup array
*  @param carr array of clusters
*  @param narr count of clusters in array
*  @param c1 pointer for saving first cluster
*  @param c2 pointer for saving second cluster
*  @pre number of clusters in array must be greater than zero
*/
void find_neighbours(struct cluster_t *carr, int narr, int *c1, int *c2)
{
    assert(narr > 0);
    float distance, new_dist;
    distance = cluster_distance(&carr[0], &carr[1]);

    for (int i = 0; i < narr; i++) {
        for (int j = i + 1; j < narr; j++)
        {
            new_dist = cluster_distance(&carr[i], &carr[j]);
            if(distance >= new_dist)
            {
                distance = new_dist;
                *c1 = i;
                *c2 = j;
            }
        }
    }
}

/**
*  Function for easier sorting
*  @ingroup cluster
*  @param a pointer to void
*  @param b pointer to void
*  @return Zero if compare is succeed
*/
static int obj_sort_compar(const void *a, const void *b)
{
    const struct obj_t *o1 = (const struct obj_t *)a;
    const struct obj_t *o2 = (const struct obj_t *)b;
    if (o1->id < o2->id) return -1;
    if (o1->id > o2->id) return 1;
    return 0;
}

/**
*  Sorting cluster by id ASC
*  @ingroup cluster
*  @param c pointer to cluster which is sorted
*/
void sort_cluster(struct cluster_t *c)
{
    qsort(c->obj, c->size, sizeof(struct obj_t), &obj_sort_compar);
}

/**
*  Prints cluster to stdout
*  @ingroup array
*  @param c pointer to cluster which is printed
*/
void print_cluster(struct cluster_t *c)
{
    for (int i = 0; i < c->size; i++)
    {
        if (i) putchar(' ');
        printf("%d[%g,%g]", c->obj[i].id, c->obj[i].x, c->obj[i].y);
    }
    putchar('\n');
}

/**
*  Loads objects from file, for each object creates cluster and inserts
*  it into an array of clusters. Also allocate space for array of Clusters
*  and pointer on first item in array saves to memory
*  @ingroup array
*  @param filename name of file from which are object loaded
*  @param arr pointer on array of clusters
*  @pre arr can't point to NULL
*  @return number of clusters in file
*/
int load_clusters(char *filename, struct cluster_t **arr)
{
    assert(arr != NULL);
    int lineNumber = 0;
    FILE *file = fopen(filename, "r");
    char line[100];
    int count;
    int id;
    float x,y;
    struct obj_t object;

    if(!file)
    {
        fprintf(stderr, "File not found\n");
        return 0;
    }

    while (fgets(line, sizeof(line), file))
    {
        if(lineNumber == 0)
        {

            if(sscanf(line, "count=%d", &count) == 0)
            {
                fprintf(stderr, "Invalid format for cluster count in file\n");
                return 0;
            }

            if(count < 1)
            {
                fprintf(stderr, "Invalid format or value for cluster count in file\n");
                return 0;
            }

            *arr = malloc(sizeof(struct cluster_t) * count);

        }
        else
        {
            init_cluster(&(*arr)[lineNumber - 1], 1);

            if(sscanf(line, "%d %f %f\n", &id, &x, &y) < 3)
            {
                fprintf(stderr, "Data are invalid\n");
                return 0;
            }

            if(0 > y || y > 1000 || 0 > x || x > 1000)
            {
                fprintf(stderr, "Data are invalid\n");
                return 0;
            }

            object.id = id;
            object.x = x;
            object.y = y;
            append_cluster(&(*arr)[lineNumber - 1], object);
        }
        lineNumber++;
    }

    if(lineNumber != count + 1)
    {
        fprintf(stderr, "Count of clusters is not equal as number in count paramteter\n");
        return 0;
    }

    fclose(file);
    return count;
}

/**
*  Prints array of clusters to stdout
*  @param carr array of clusters
*  @param narr count of clusters in array
*/
void print_clusters(struct cluster_t *carr, int narr)
{
    printf("Clusters:\n");
    for (int i = 0; i < narr; i++)
    {
        printf("cluster %d: ", i);
        print_cluster(&carr[i]);
    }
}

/**
*  Main function
*  @param argc number of arguments
*  @param argv array of arguments
*  @return zero if program is successful
*/
int main(int argc, char *argv[])
{
    struct cluster_t *clusters;
    int size;
    int narr = 1;

    if(argc < 2)
    {
        fprintf(stderr, "Filename is not set\n");
        return -1;
    }

    if(argc > 3)
    {

        premium_case = 0;

        if(!strcmp(argv[3], "--min"))
            premium_case = 1;

        if(!strcmp(argv[3], "--max"))
            premium_case = 2;

    }

    if(argc > 2)
    {
        char *fail;
        narr = strtol(argv[2], &fail, 10);

        if(strlen(fail) != 0){
            fprintf(stderr, "Invalid argument of program\n");
            return -1;
        }

        if(narr <= 0)
        {
            fprintf(stderr, "Invalid cluster count\n");
            return -1;
        }
    }

    size = load_clusters(argv[1], &clusters);

    if(size == 0)
    {
        return -1;
    }

    if(narr > size && size != 0)
    {
        fprintf(stderr, "Argument is greater that count of clusters\n");
        return -1;
    }

    int c1,c2;

    while(size > narr)
    {
        find_neighbours(clusters, size, &c1, &c2);
        merge_clusters(&clusters[c1], &clusters[c2]);
        remove_cluster(clusters, size, c2);
        size--;
    }

    print_clusters(clusters, size);

    for(int i = 0; i < size; i++)
        clear_cluster(&clusters[i]);

    free(clusters);

    return 0;
}
