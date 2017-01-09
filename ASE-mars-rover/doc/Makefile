SRCS = $(wildcard *.md)
ALL_HTML = $(patsubst %.md,%.html,$(SRCS))
ALL_PDF = $(patsubst %.md,%.pdf,$(SRCS))

all: $(ALL_HTML) $(ALL_PDF) 

%.html: %.md
	pandoc -s -S --toc -c pandoc.css --mathjax -f markdown $< -o $@

%.tex: %.md
	pandoc -s -S --toc -c pandoc.css --mathjax -f markdown $< -o $@

%.pdf: %.tex
	pdflatex $< 
	pdflatex $< 

publish: Projet.pdf Projet.html 
	scp -r Projet.pdf Projet.html pandoc.css images/ tahiti.prism.uvsq.fr:/var/www/ase/projet/

clean:
	rm -f $(ALL_HTML) $(ALL_PDF) *.aux *.log *.toc *.out 
