#include <stdio.h>
#include "bmp_header.h"
#include <stdlib.h>
#include <string.h>
#include <math.h>

typedef struct
{
	unsigned char r, g, b;
} pixel;

typedef struct
{
	int line, col;
} stack;

pixel **get_image (signed int width, signed int height)
{
	int i, j;
	pixel **matrix;

	matrix = (pixel **) malloc (height * sizeof(pixel *));
	if (matrix == NULL)
	{
		return NULL;
	}

	for (i = 0; i < height; i++)
	{
		matrix[i] = (pixel *) calloc (width, sizeof(pixel));
		if (matrix[i] == NULL)
		{
			for (j = 0; j < i; j++)
			{
				free (matrix[j]);
			}
			free (matrix);
			return NULL;
		}
	}

	return matrix;
}

void free_matrix (pixel ***mat, int n)
{
	int i;

	for (i = 0; i < n; i++)
	{
		free ((*mat)[i]);
	}
	free (*mat);
	*mat = NULL;
}

int **zone (signed int width, signed int height)
{
	int i, j;
	int **matrix;

	matrix = (int **) malloc (height * sizeof(int *));
	if (matrix == NULL)
	{
		return NULL;
	}

	for (i = 0; i < height; i++)
	{
		matrix[i] = (int *) calloc (width, sizeof(int));
		if (matrix[i] == NULL)
		{
			for (j = 0; j < i; j++)
			{
				free (matrix[j]);
			}
			free (matrix);
			return NULL;
		}
	}

	return matrix;
}

void free_zone (int ***mat, int n)
{
	int i;

	for (i = 0; i < n; i++)
	{
		free ((*mat)[i]);
	}
	free (*mat);
	*mat = NULL;
}

int main ()
{
	//citire valori input.txt
	char input_file[] = "input.txt";
	FILE *input = fopen (input_file, "rt");

	if (input == NULL)
	{
		fprintf (stderr, "Can't open file %s", input_file);
		return -1;
	}

	char image_name[50], fisbin_name[50];
	int value;

	fscanf (input, "%s", image_name);
	fscanf (input, "%d", &value);
	fscanf (input, "%s", fisbin_name);

	fclose(input);
	//sfarsitul citirii valorilor input.txt

	//citire fisier bmp
	FILE *bmp_image = fopen (image_name, "rb");

	if (bmp_image == NULL)
	{
		fprintf (stderr, "Can't open file %s", image_name);
		return -1;
	}

	//declarare variabile pentru structurile bmp
	bmp_fileheader fheader;
	bmp_infoheader iheader;

	//citire fileheader
	fread (&fheader, sizeof(bmp_fileheader), 1, bmp_image);
	fseek (bmp_image, sizeof(bmp_fileheader), SEEK_SET);

	//citire infoheader
	fread (&iheader, sizeof(bmp_infoheader), 1, bmp_image);
	fseek(bmp_image, fheader.imageDataOffset, SEEK_SET);

	pixel **image;
	image = get_image (iheader.width, iheader.height);

	int i, j;
	signed int padding = 0;

	for (i = 0; i < iheader.height; i++)
	{
		for (j = 0; j < iheader.width; j++)
		{
			fread (&image[i][j].b,
			sizeof(unsigned char), 1, bmp_image);
			fread (&image[i][j].g,
			sizeof(unsigned char), 1, bmp_image);
			fread (&image[i][j].r,
			sizeof(unsigned char), 1, bmp_image);
		}
		//verificare padding
		if (iheader.width % 4 != 0)
		{
			padding = iheader.width + (4 - iheader.width % 4);
			fseek(bmp_image, padding-iheader.width *
			sizeof(unsigned char), SEEK_CUR);
		}
	}

	fclose(bmp_image);
	//sfarsitul citirii fisierului bmp

	//transformare matrice noua
	pixel **bw_image;
	bw_image = get_image (iheader.width, iheader.height);

	unsigned char color;
	for (i = 0; i < iheader.height; i++)
	{
		for (j = 0; j < iheader.width; j++)
		{
			color = (image[i][j].b +
			image[i][j].g + image[i][j].r) / 3;
			bw_image[i][j].b = color;
			bw_image[i][j].g = color;
			bw_image[i][j].r = color;
		}
	}
	//sfarsitul transformarii noii matrice

	//denumire fisier nou
	char bwout_name[100] = "";
	i = 0;
	while(image_name[i] != '.')
	{
		bwout_name[i] = image_name[i];
		i++;
	}
	strcat (bwout_name, "_black_white.bmp");

	FILE *bwoutput = fopen(bwout_name, "wb");

	if (bwoutput == NULL)
	{
		fprintf(stderr, "Can't open %s", bwout_name);
		return -1;
	}

	//afisare fileheader
	fwrite (&fheader, sizeof(bmp_fileheader), 1, bwoutput);
	fseek (bwoutput, sizeof(bmp_fileheader), SEEK_SET);

	//afisare infoheader
	fwrite (&iheader, sizeof(bmp_infoheader), 1, bwoutput);
	fseek(bwoutput, sizeof(bmp_infoheader), SEEK_CUR);

	fseek(bwoutput, fheader.imageDataOffset, SEEK_SET);

	//afisare imagine black&white
	for(i = 0; i < iheader.height; i++)
	{
		for(j = 0; j < iheader.width; j++)
		{
			fwrite (&bw_image[i][j].b,
			sizeof(unsigned char), 1, bwoutput);
			fwrite (&bw_image[i][j].g,
			sizeof(unsigned char), 1, bwoutput);
			fwrite (&bw_image[i][j].r,
			sizeof(unsigned char), 1, bwoutput);
		}
	}
	fclose(bwoutput);

	pixel **mbw_image;
	mbw_image = get_image(iheader.width + 2, iheader.height + 2);

	//adaugarea marginilor la matricea bw
	for(i = 1; i <= iheader.height; i++)
	{
		for(j = 1; j <= iheader.width; j++)
		{
			mbw_image[iheader.height-i+1][j] = bw_image[i-1][j-1];
		}
	}

	//declarare matrici filtre
	int F1[3][3], F2[3][3], F3[3][3];
	F1[0][0] = F1[0][1] = F1[0][2] = F1[1][0] = -1;
	F1[2][0] = F1[2][1] = F1[1][2] = F1[2][2] = -1;
	F1[1][1] = 8;

	F2[0][0] = F2[0][2] = F2[2][0] = F2[2][2] = 0;
	F2[1][0] = F2[0][1] = F2[2][1] = F2[1][2] = 1;
	F2[1][1] = -4;

	F3[1][0] = F3[0][1] = F3[1][1] = F3[1][2] = F3[2][1] = 0;
	F3[0][0] = F3[2][2] = 1;
	F3[2][0] = F3[0][2] = -1;
	//sfarsit declarare matrici filtre

	//denumirea noilor imagini
	char f1_name[100] = "";
	char f2_name[100] = "";
	char f3_name[100] = "";

	i = 0;
	while(image_name[i] != '.')
	{
		f1_name[i] = image_name[i];
		i++;
	}
	strcat(f1_name, "_f1.bmp");

	i = 0;
	while(image_name[i] != '.')
	{
		f2_name[i] = image_name[i];
		i++;
	}
	strcat(f2_name, "_f2.bmp");

	i = 0;
	while(image_name[i] != '.')
	{
		f3_name[i] = image_name[i];
		i++;
	}
	strcat(f3_name, "_f3.bmp");
	//sfarsitul denumirii noilor imagini


	pixel **f1_matrix, **f2_matrix, **f3_matrix;

	int sumr = 0, sumg = 0, sumb = 0;

	f1_matrix = get_image (iheader.width + 2, iheader.height + 2);
	f2_matrix = get_image (iheader.width + 2, iheader.height + 2);
	f3_matrix = get_image (iheader.width + 2, iheader.height + 2);

	for(i = 1; i <= iheader.height; i++)
	{
		for(j = 1; j <= iheader.width; j++)
		{
			sumb = 0; sumg = 0; sumr = 0;

			sumb += mbw_image[i-1][j-1].b * F1[0][0];
			sumb += mbw_image[i-1][j].b * F1[0][1];
			sumb += mbw_image[i-1][j+1].b * F1[0][2];
			sumb += mbw_image[i][j-1].b * F1[1][0];
			sumb += mbw_image[i][j].b * F1[1][1];
			sumb += mbw_image[i][j+1].b * F1[1][2];
			sumb += mbw_image[i+1][j-1].b * F1[2][0];
			sumb += mbw_image[i+1][j].b * F1[2][1];
			sumb += mbw_image[i+1][j+1].b * F1[2][2];

			if (sumb < 0)
			{
				f1_matrix[i][j].b = 0;
			}
			else if (sumb > 255)
			{
				f1_matrix[i][j].b = 255;
			}
			else
			{
				f1_matrix[i][j].b = (unsigned char) sumb;
			}

			sumg += mbw_image[i-1][j-1].g * F1[0][0];
			sumg += mbw_image[i-1][j].g * F1[0][1];
			sumg += mbw_image[i-1][j+1].g * F1[0][2];
			sumg += mbw_image[i][j-1].g * F1[1][0];
			sumg += mbw_image[i][j].g * F1[1][1];
			sumg += mbw_image[i][j+1].g * F1[1][2];
			sumg += mbw_image[i+1][j-1].g * F1[2][0];
			sumg += mbw_image[i+1][j].g * F1[2][1];
			sumg += mbw_image[i+1][j+1].g * F1[2][2];

			if(sumg < 0)
			{
				f1_matrix[i][j].g = 0;
			}
			else if (sumg > 255)
			{
				f1_matrix[i][j].g = 255;
			}
			else
			{
				f1_matrix[i][j].g = (unsigned char) sumg;
			}

			sumr += mbw_image[i-1][j-1].r * F1[0][0];
			sumr += mbw_image[i-1][j].r * F1[0][1];
			sumr += mbw_image[i-1][j+1].r * F1[0][2];
			sumr += mbw_image[i][j-1].r * F1[1][0];
			sumr += mbw_image[i][j].r * F1[1][1];
			sumr += mbw_image[i][j+1].r * F1[1][2];
			sumr += mbw_image[i+1][j-1].r * F1[2][0];
			sumr += mbw_image[i+1][j].r * F1[2][1];
			sumr += mbw_image[i+1][j+1].r * F1[2][2];

			if (sumr < 0)
			{
				f1_matrix[i][j].r = 0;
			}
			else if (sumr > 255)
			{
				f1_matrix[i][j].r = 255;
			}
			else
			{
				f1_matrix[i][j].r = (unsigned char) sumr;
			}

		}
	}

	FILE *f1output = fopen(f1_name, "wb");

	if (f1output == NULL)
	{
		fprintf(stderr, "Can't open %s", f1_name);
		return -1;
	}

	//afisare fileheader
	fwrite (&fheader, sizeof(bmp_fileheader), 1, f1output);
	fseek (f1output, sizeof(bmp_fileheader), SEEK_SET);

	//afisare infoheader
	fwrite (&iheader, sizeof(bmp_infoheader), 1, f1output);
	fseek(f1output, sizeof(bmp_infoheader), SEEK_CUR);

	fseek(f1output, fheader.imageDataOffset, SEEK_SET);

	//afisare imagine
	for(i = iheader.height; i >= 1; i--)
	{
		for(j = 1; j <= iheader.width; j++)
		{
			fwrite (&f1_matrix[i][j].b,
			sizeof(unsigned char), 1, f1output);
			fwrite (&f1_matrix[i][j].g,
			sizeof(unsigned char), 1, f1output);
			fwrite (&f1_matrix[i][j].r,
			sizeof(unsigned char), 1, f1output);
		}
	}
	fclose(f1output);

	for(i = 1; i <= iheader.height; i++)
	{
		for(j = 1; j <= iheader.width; j++)
		{
			sumb = 0; sumg = 0; sumr = 0;

			sumb += mbw_image[i-1][j-1].b * F2[0][0];
			sumb += mbw_image[i-1][j].b * F2[0][1];
			sumb += mbw_image[i-1][j+1].b * F2[0][2];
			sumb += mbw_image[i][j-1].b * F2[1][0];
			sumb += mbw_image[i][j].b * F2[1][1];
			sumb += mbw_image[i][j+1].b * F2[1][2];
			sumb += mbw_image[i+1][j-1].b * F2[2][0];
			sumb += mbw_image[i+1][j].b * F2[2][1];
			sumb += mbw_image[i+1][j+1].b * F2[2][2];

			if (sumb < 0)
			{
				f2_matrix[i][j].b = 0;
			}
			else if (sumb > 255)
			{
				f2_matrix[i][j].b = 255;
			}
			else
			{
				f2_matrix[i][j].b = (unsigned char) sumb;
			}

			sumg += mbw_image[i-1][j-1].g * F2[0][0];
			sumg += mbw_image[i-1][j].g * F2[0][1];
			sumg += mbw_image[i-1][j+1].g * F2[0][2];
			sumg += mbw_image[i][j-1].g * F2[1][0];
			sumg += mbw_image[i][j].g * F2[1][1];
			sumg += mbw_image[i][j+1].g * F2[1][2];
			sumg += mbw_image[i+1][j-1].g * F2[2][0];
			sumg += mbw_image[i+1][j].g * F2[2][1];
			sumg += mbw_image[i+1][j+1].g * F2[2][2];

			if(sumg < 0)
			{
				f2_matrix[i][j].g = 0;
			}
			else if (sumg > 255)
			{
				f2_matrix[i][j].g = 255;
			}
			else
			{
				f2_matrix[i][j].g = (unsigned char) sumg;
			}

			sumr += mbw_image[i-1][j-1].r * F2[0][0];
			sumr += mbw_image[i-1][j].r * F2[0][1];
			sumr += mbw_image[i-1][j+1].r * F2[0][2];
			sumr += mbw_image[i][j-1].r * F2[1][0];
			sumr += mbw_image[i][j].r * F2[1][1];
			sumr += mbw_image[i][j+1].r * F2[1][2];
			sumr += mbw_image[i+1][j-1].r * F2[2][0];
			sumr += mbw_image[i+1][j].r * F2[2][1];
			sumr += mbw_image[i+1][j+1].r * F2[2][2];

			if (sumr < 0)
			{
				f2_matrix[i][j].r = 0;
			}
			else if (sumr > 255)
			{
				f2_matrix[i][j].r = 255;
			}
			else
			{
				f2_matrix[i][j].r = (unsigned char) sumr;
			}

		}
	}

	FILE *f2output = fopen(f2_name, "wb");

	if (f2output == NULL)
	{
		fprintf(stderr, "Can't open %s", f2_name);
		return -1;
	}

	//afisare fileheader
	fwrite (&fheader, sizeof(bmp_fileheader), 1, f2output);
	fseek (f2output, sizeof(bmp_fileheader), SEEK_SET);

	//afisare infoheader
	fwrite (&iheader, sizeof(bmp_infoheader), 1, f2output);
	fseek(f2output, sizeof(bmp_infoheader), SEEK_CUR);

	fseek(f2output, fheader.imageDataOffset, SEEK_SET);

	//afisare imagine
	for(i = iheader.height; i >= 1; i--)
	{
		for(j = 1; j <= iheader.width; j++)
		{
			fwrite (&f2_matrix[i][j].b,
			sizeof(unsigned char), 1, f2output);
			fwrite (&f2_matrix[i][j].g,
			sizeof(unsigned char), 1, f2output);
			fwrite (&f2_matrix[i][j].r,
			sizeof(unsigned char), 1, f2output);
		}
	}
	fclose(f2output);

	for(i = 1; i <= iheader.height; i++)
	{
		for(j = 1; j <= iheader.width; j++)
		{
			sumb = 0; sumg = 0; sumr = 0;

			sumb += mbw_image[i-1][j-1].b * F3[0][0];
			sumb += mbw_image[i-1][j].b * F3[0][1];
			sumb += mbw_image[i-1][j+1].b * F3[0][2];
			sumb += mbw_image[i][j-1].b * F3[1][0];
			sumb += mbw_image[i][j].b * F3[1][1];
			sumb += mbw_image[i][j+1].b * F3[1][2];
			sumb += mbw_image[i+1][j-1].b * F3[2][0];
			sumb += mbw_image[i+1][j].b * F3[2][1];
			sumb += mbw_image[i+1][j+1].b * F3[2][2];

			if (sumb < 0)
			{
				f3_matrix[i][j].b = 0;
			}
			else if (sumb > 255)
			{
				f3_matrix[i][j].b = 255;
			}
			else
			{
				f3_matrix[i][j].b = (unsigned char) sumb;
			}

			sumg += mbw_image[i-1][j-1].g * F3[0][0];
			sumg += mbw_image[i-1][j].g * F3[0][1];
			sumg += mbw_image[i-1][j+1].g * F3[0][2];
			sumg += mbw_image[i][j-1].g * F3[1][0];
			sumg += mbw_image[i][j].g * F3[1][1];
			sumg += mbw_image[i][j+1].g * F3[1][2];
			sumg += mbw_image[i+1][j-1].g * F3[2][0];
			sumg += mbw_image[i+1][j].g * F3[2][1];
			sumg += mbw_image[i+1][j+1].g * F3[2][2];

			if(sumg < 0)
			{
				f3_matrix[i][j].g = 0;
			}
			else if (sumg > 255)
			{
				f3_matrix[i][j].g = 255;
			}
			else
			{
				f3_matrix[i][j].g = (unsigned char) sumg;
			}

			sumr += mbw_image[i-1][j-1].r * F3[0][0];
			sumr += mbw_image[i-1][j].r * F3[0][1];
			sumr += mbw_image[i-1][j+1].r * F3[0][2];
			sumr += mbw_image[i][j-1].r * F3[1][0];
			sumr += mbw_image[i][j].r * F3[1][1];
			sumr += mbw_image[i][j+1].r * F3[1][2];
			sumr += mbw_image[i+1][j-1].r * F3[2][0];
			sumr += mbw_image[i+1][j].r * F3[2][1];
			sumr += mbw_image[i+1][j+1].r * F3[2][2];

			if (sumr < 0)
			{
				f3_matrix[i][j].r = 0;
			}
			else if (sumr > 255)
			{
				f3_matrix[i][j].r = 255;
			}
			else
			{
				f3_matrix[i][j].r = (unsigned char) sumr;
			}

		}
	}

	FILE *f3output = fopen(f3_name, "wb");

	if (f3output == NULL)
	{
		fprintf(stderr, "Can't open %s", f3_name);
		return -1;
	}

	//afisare fileheader
	fwrite (&fheader, sizeof(bmp_fileheader), 1, f3output);
	fseek (f3output, sizeof(bmp_fileheader), SEEK_SET);

	//afisare infoheader
	fwrite (&iheader, sizeof(bmp_infoheader), 1, f3output);
	fseek(f3output, sizeof(bmp_infoheader), SEEK_CUR);

	fseek(f3output, fheader.imageDataOffset, SEEK_SET);

	//afisare imagine
	for(i = iheader.height; i >= 1; i--)
	{
		for(j = 1; j <= iheader.width; j++)
		{
			fwrite (&f3_matrix[i][j].b,
			sizeof(unsigned char), 1, f3output);
			fwrite (&f3_matrix[i][j].g,
			sizeof(unsigned char), 1, f3output);
			fwrite (&f3_matrix[i][j].r,
			sizeof(unsigned char), 1, f3output);
		}
	}
	fclose(f3output);

	free_matrix(&bw_image, iheader.height);
	free_matrix(&mbw_image, iheader.height + 2);
	free_matrix(&f1_matrix, iheader.height + 2);
	free_matrix(&f2_matrix, iheader.height + 2);
	free_matrix(&f3_matrix, iheader.height + 2);

	int **zone_matrix;
	zone_matrix = zone (iheader.width, iheader.height);

	pixel **reverse_image = get_image(iheader.width, iheader.height);
	for(i = 0; i < iheader.height; i++)
	{
		for(j = 0; j < iheader.width; j++)
		{
			reverse_image[i][j] = image[iheader.height - i - 1][j];
		}
	}

//----> Aici incepe algoritmul de compresie
int lnr = 0, cnr = 0, poz1 = 0, poz2 = 0;
int n = 1;
int ok = 1;
int c = 0;
stack *s = NULL;
pixel pcolor;

for(i = 0; i < iheader.height; i++)
{
	j = 0;
	while(j < iheader.width)
	{
		if(zone_matrix[i][j] == 0)
		{
			n = 1;
			c++;
			poz1 = i;
			poz2 = j;
			s = calloc(n, sizeof(stack));
			s[0].line = poz1;
			s[0].col = poz2;
			zone_matrix[poz1][poz2] = c;
			pcolor = reverse_image[poz1][poz2];

			while (n > 0)
			{
				lnr = s[n-1].line;
				cnr = s[n-1].col;
				n--;
				s = realloc (s, n * sizeof(stack));

				if (lnr - 1 > -1)
				if(zone_matrix[lnr-1][cnr] == 0)
				if ((abs(pcolor.r - reverse_image[lnr-1][cnr].r)
				+ abs(pcolor.g - reverse_image[lnr-1][cnr].g)
				+ abs(pcolor.b - reverse_image[lnr-1][cnr].b))
				<= value)
				{
					zone_matrix[lnr-1][cnr] = c;
					reverse_image[lnr-1][cnr] = pcolor;
					n++;
					s = realloc(s, n * sizeof(stack));
					s[n-1].line = lnr - 1;
					s[n-1].col = cnr;
				}

				if (lnr + 1 < iheader.height)
				if(zone_matrix[lnr+1][cnr] == 0)
				if ((abs(pcolor.r - reverse_image[lnr+1][cnr].r)
				+ abs(pcolor.g - reverse_image[lnr+1][cnr].g)
				+ abs(pcolor.b - reverse_image[lnr+1][cnr].b))
				<= value)
				{
					zone_matrix[lnr+1][cnr] = c;
					reverse_image[lnr+1][cnr] = pcolor;
					n++;
					s = realloc(s, n * sizeof(stack));
					s[n-1].line = lnr + 1;
					s[n-1].col = cnr;
				}

				if (cnr - 1 > -1)
				if(zone_matrix[lnr][cnr-1] == 0)
				if ((abs(pcolor.r - reverse_image[lnr][cnr-1].r)
				+ abs(pcolor.g - reverse_image[lnr][cnr-1].g)
				+ abs(pcolor.b - reverse_image[lnr][cnr-1].b))
				<= value)
				{
					zone_matrix[lnr][cnr-1] = c;
					reverse_image[lnr][cnr-1] = pcolor;
					n++;
					s = realloc(s, n * sizeof(stack));
					s[n-1].line = lnr;
					s[n-1].col = cnr - 1;
				}

				if (cnr + 1 < iheader.width)
				if(zone_matrix[lnr][cnr+1] == 0)
				if ((abs(pcolor.r - reverse_image[lnr][cnr+1].r)
				+ abs(pcolor.g - reverse_image[lnr][cnr+1].g)
				+ abs(pcolor.b - reverse_image[lnr][cnr+1].b))
				<= value)
				{
					zone_matrix[lnr][cnr+1] = c;
					reverse_image[lnr][cnr+1] = pcolor;
					n++;
					s = realloc(s, n * sizeof(stack));
					s[n-1].line = lnr;
					s[n-1].col = cnr + 1;
				}
			}
		}
		j++;

	}
}
//-----> Aici se incheie


	char compress[] = "compressed.bin";

	FILE *com = fopen(compress, "wb");

	if (com == NULL)
	{
		fprintf(stderr, "Can't open %s", compress);
		return -1;
	}

	//afisare fileheader
	fwrite (&fheader, sizeof(bmp_fileheader), 1, com);
	fseek (com, sizeof(bmp_fileheader), SEEK_SET);

	//afisare infoheader
	fwrite (&iheader, sizeof(bmp_infoheader), 1, com);
	fseek(com, sizeof(bmp_infoheader), SEEK_CUR);

	fseek(com, fheader.imageDataOffset, SEEK_SET);

	unsigned short nr_line = 0, nr_col = 0;

	for(i = 0; i < iheader.height; i++)
	{
		for(j = 0; j < iheader.width; j++)
		{
			if (i == 0 || i == iheader.height - 1 ||
			j == 0 || j == iheader.width - 1)
			{
				nr_line = i+1; nr_col = j+1;
				fwrite(&(nr_line),
				sizeof(unsigned short), 1, com);
				fwrite(&(nr_col),
				sizeof(unsigned short), 1, com);
				fwrite(&reverse_image[i][j].r,
				sizeof(unsigned char), 1, com);
				fwrite(&reverse_image[i][j].g,
				sizeof(unsigned char), 1, com);
				fwrite(&reverse_image[i][j].b,
				sizeof(unsigned char), 1, com);
			}
			else if (zone_matrix[i+1][j] != zone_matrix[i][j] ||
			zone_matrix[i-1][j] != zone_matrix[i][j] ||
			zone_matrix[i][j+1] != zone_matrix[i][j] ||
			zone_matrix[i][j-1] != zone_matrix[i][j])
			{
				nr_line = i+1; nr_col = j+1;
				fwrite(&(nr_line),
				sizeof(unsigned short), 1, com);
				fwrite(&(nr_col),
				sizeof(unsigned short), 1, com);
				fwrite(&reverse_image[i][j].r,
				sizeof(unsigned char), 1, com);
				fwrite(&reverse_image[i][j].g,
				sizeof(unsigned char), 1, com);
				fwrite(&reverse_image[i][j].b,
				sizeof(unsigned char), 1, com);

			}
		}
	}

	free_matrix(&reverse_image, iheader.height);
	free_matrix(&image, iheader.height);
	free_zone(&zone_matrix, iheader.height);

	unsigned char red, green, blue;
	unsigned short i1 = 1, j1 = 1, i2 = 1, j2 = 1;
	FILE *decom = fopen (fisbin_name, "rb");

	if (decom == NULL)
	{
		fprintf (stderr, "Can't open file %s", fisbin_name);
		return -1;
	}

	//citire fileheader
	fread (&fheader, sizeof(bmp_fileheader), 1, decom);
	fseek (decom, sizeof(bmp_fileheader), SEEK_SET);

	//citire infoheader
	fread (&iheader, sizeof(bmp_infoheader), 1, decom);
	fseek(decom, fheader.imageDataOffset, SEEK_SET);

	pixel **decompressed;
	decompressed = get_image(iheader.width, iheader.height);

	while(feof(decom) == 0)
	{
		fread(&i1, sizeof(unsigned short), 1, decom);
		fread(&j1, sizeof(unsigned short), 1, decom);
		fread(&red, sizeof(unsigned char), 1, decom);
		fread(&green, sizeof(unsigned char), 1, decom);
		fread(&blue, sizeof(unsigned char),1, decom);

		if(i1 == i2 && (j1 - j2) > 1)
		{
			j = j2;
			while (j < j1)
			{
				decompressed[i1-1][j].r = red;
				decompressed[i1-1][j].g = green;
				decompressed[i1-1][j].b = blue;
				j++;
			}
		}

		else
		{
			decompressed[i1-1][j1-1].r = red;
			decompressed[i1-1][j1-1].g = green;
			decompressed[i1-1][j1-1].b = blue;
		}

		i2 = i1; j2 = j1;
	}

	fclose(decom);

	char decompressed_name[] = "decompressed.bmp";
	FILE *dec = fopen(decompressed_name, "wb");

	if (dec == NULL)
	{
		fprintf(stderr, "Can't open %s", decompressed_name);
		return -1;
	}

	//afisare fileheader
	fwrite (&fheader, sizeof(bmp_fileheader), 1, dec);
	fseek (dec, sizeof(bmp_fileheader), SEEK_SET);

	//afisare infoheader
	fwrite (&iheader, sizeof(bmp_infoheader), 1, dec);
	fseek(dec, sizeof(bmp_infoheader), SEEK_CUR);

	fseek(dec, fheader.imageDataOffset, SEEK_SET);

	//afisare imagine
	for(i = iheader.height - 1; i >= 0; i--)
	{
		for(j = 0; j <= iheader.width - 1; j++)
		{
			fwrite (&decompressed[i][j].b,
			sizeof(unsigned char), 1, dec);
			fwrite (&decompressed[i][j].g,
			sizeof(unsigned char), 1, dec);
			fwrite (&decompressed[i][j].r,
			sizeof(unsigned char), 1, dec);
		}
	}
	fclose(dec);

	free_matrix(&decompressed, iheader.height);

	return 0;
}
