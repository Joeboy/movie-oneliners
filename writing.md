# Take a fountain file and convert it to pdf (via html). Fountain is a plain
# text format for script writing: http://fountain.io/ , https://github.com/olivertaylor/Textplay
# Fountain format is great because, being plain text, you can use standard tools like git for
# versioning, collaboration etc.

    cat myfile.fountain | textplay > /tmp/tp-out.html && wkhtmltopdf /tmp/tp-out.html myfile.pdf
