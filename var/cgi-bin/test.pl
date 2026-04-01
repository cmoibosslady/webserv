#!/usr/bin/perl
use strict;
use warnings;

sub escape_html {
    my ($text) = @_;
    $text = '' unless defined $text;
    $text =~ s/&/&amp;/g;
    $text =~ s/</&lt;/g;
    $text =~ s/>/&gt;/g;
    return $text;
}

my $method = $ENV{'REQUEST_METHOD'} // '';
my $query = $ENV{'QUERY_STRING'} // '';
my $content_type = $ENV{'CONTENT_TYPE'} // '';
my $content_length = $ENV{'CONTENT_LENGTH'} // '';

my $body = '';
if ($method eq 'POST' && $content_length =~ /^\d+$/ && $content_length > 0) {
    read(STDIN, $body, $content_length);
}

my $now = scalar localtime();

print "Status: 200 OK\r\n";
print "Content-Type: text/html; charset=UTF-8\r\n";
print "Set-Cookie: cgi_perl_test=ok; Path=/\r\n\r\n";

print "<!doctype html>\n";
print "<html lang=\"fr\">\n";
print "<head><meta charset=\"utf-8\"><title>CGI Perl Test</title></head>\n";
print "<body>\n";
print "  <h1>CGI Perl - OK</h1>\n";
print "  <p>Ce script confirme l'exécution CGI en <strong>perl</strong>.</p>\n";
print "\n";
print "  <h2>Request</h2>\n";
print "  <ul>\n";
print "    <li><strong>DATE</strong>: " . escape_html($now) . "</li>\n";
print "    <li><strong>REQUEST_METHOD</strong>: " . escape_html($method) . "</li>\n";
print "    <li><strong>SCRIPT_NAME</strong>: " . escape_html($ENV{'SCRIPT_NAME'} // '') . "</li>\n";
print "    <li><strong>QUERY_STRING</strong>: " . escape_html($query) . "</li>\n";
print "    <li><strong>CONTENT_TYPE</strong>: " . escape_html($content_type) . "</li>\n";
print "    <li><strong>CONTENT_LENGTH</strong>: " . escape_html($content_length) . "</li>\n";
print "  </ul>\n";
print "\n";
print "  <h2>Payload POST</h2>\n";
print "  <pre>" . escape_html($body) . "</pre>\n";
print "\n";
print "  <h2>Headers utiles</h2>\n";
print "  <ul>\n";
print "    <li><strong>HTTP_HOST</strong>: " . escape_html($ENV{'HTTP_HOST'} // '') . "</li>\n";
print "    <li><strong>HTTP_USER_AGENT</strong>: " . escape_html($ENV{'HTTP_USER_AGENT'} // '') . "</li>\n";
print "    <li><strong>REMOTE_ADDR</strong>: " . escape_html($ENV{'REMOTE_ADDR'} // '') . "</li>\n";
print "    <li><strong>HTTP_COOKIE</strong>: " . escape_html($ENV{'HTTP_COOKIE'} // '') . "</li>\n";
print "  </ul>\n";
print "</body>\n";
print "</html>\n";
