#BHEADER***********************************************************************
# (c) 1997   The Regents of the University of California
#
# See the file COPYRIGHT_and_DISCLAIMER for a complete copyright
# notice, contact person, and disclaimer.
#
# $Revision$
#EHEADER***********************************************************************

.SUFFIXES:
.SUFFIXES: .c .f .o

HEADERS =\
 box.h\
 box_neighbors.h\
 communication.h\
 computation.h\
 general.h\
 headers.h\
 protos.h\
 sbox.h\
 struct_grid.h\
 struct_matrix.h\
 struct_stencil.h\
 struct_vector.h\
 HYPRE_mv.h

FILES =\
 box.c\
 box_algebra.c\
 box_neighbors.c\
 communication.c\
 communication_info.c\
 computation.c\
 grow.c\
 project.c\
 sbox.c\
 struct_grid.c\
 struct_io.c\
 struct_axpy.c\
 struct_copy.c\
 struct_innerprod.c\
 struct_matrix.c\
 struct_matrix_mask.c\
 struct_matvec.c\
 struct_scale.c\
 struct_stencil.c\
 struct_vector.c\
 HYPRE_struct_grid.c\
 HYPRE_struct_matrix.c\
 HYPRE_struct_stencil.c\
 HYPRE_struct_vector.c

OBJS = ${FILES:.c=.o}

CC = cicc
F77 = ci77

# CFLAGS = -O
CFLAGS = -O -DHYPRE_TIMING -DHYPRE_COMM_SIMPLE
# CFLAGS = -g -DHYPRE_TIMING -DHYPRE_COMM_SIMPLE

FFLAGS = -O

LFLAGS =\
 -L/usr/local/lib\
 -L.\
 -L../utilities\
 -lHYPRE_mv\
 -lHYPRE_timing\
 -lHYPRE_memory\
 -lmpi\
 -lm


##################################################################
# Main rules
##################################################################

driver_internal: driver_internal.o libHYPRE_mv.a
	@echo  "Linking" $@ "... "
	${CC} -o driver_internal driver_internal.o ${LFLAGS}

test_internal: test_internal.o libHYPRE_mv.a
	@echo  "Linking" $@ "... "
	${CC} -o test_internal test_internal.o ${LFLAGS}

one_to_many: one_to_many.o libHYPRE_mv.a
	@echo  "Linking" $@ "... "
	${CC} -o one_to_many one_to_many.o ${LFLAGS}

one_to_many_vector: one_to_many_vector.o libHYPRE_mv.a
	@echo  "Linking" $@ "... "
	${CC} -o one_to_many_vector one_to_many_vector.o ${LFLAGS}

create_2d_laplacian: create_2d_laplacian.o libHYPRE_mv.a
	@echo  "Linking" $@ "... "
	${CC} -o create_2d_laplacian create_2d_laplacian.o ${LFLAGS}

create_3d_laplacian: create_3d_laplacian.o libHYPRE_mv.a
	@echo  "Linking" $@ "... "
	${CC} -o create_3d_laplacian create_3d_laplacian.o ${LFLAGS}

libHYPRE_mv.a: ${OBJS}
	@echo  "Building $@ ... "
	ar -ru $@ ${OBJS}
	ranlib $@

${OBJS}: ${HEADERS}

##################################################################
# Generic rules
##################################################################

.c.o:
	@echo "Making (c) " $@
	@${CC} -o $@ -c ${CFLAGS} $<

.f.${AMG_ARCH}.o:
	@echo "Making (f) " $@
	@${F77} -o $@ -c ${FFLAGS} $<

##################################################################
# Miscellaneous rules
##################################################################

veryclean: clean
	@rm -f libHYPRE_mv.a
	@rm -f driver driver_internal
	@rm -f one_to_many
	@rm -f one_to_many_vector
	@rm -f create_2d_laplacian
	@rm -f create_3d_laplacian

clean:
	@rm -f *.o

