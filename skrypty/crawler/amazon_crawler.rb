require 'nokogiri'
require 'httparty'
# require 'byebug'

def crawler
    # default amazon.pl page if search not specified in ARGV
    url = "https://www.amazon.pl/s?k=deep+learning+book&__mk_pl_PL=%C3%85M%C3%85%C5%BD%C3%95%C3%91&ref=nb_sb_noss"
    if !ARGV.empty?
        url = "https://www.amazon.pl/"
        url += ARGV[0]
        for arg in ARGV[1..]
            url += "-" + arg
        end
        url = url + "/s?k=" + ARGV[0]
        for arg in ARGV[1..]
            url += "+" + arg
        end
    end
    items = Array.new
    while true
        unparsed_page = HTTParty.get(url)
        parsed_page = Nokogiri::HTML(unparsed_page)
        item_listings = parsed_page.css('div.s-card-container.s-overflow-hidden.s-expand-height.s-include-content-margin.s-latency-cf-section.s-card-border')
        item_listings.each do |item_listing|
            item = {
                title: item_listing.css('span.a-size-base-plus.a-color-base.a-text-normal').text,
                price: item_listing.css('span.a-price-whole').text + item_listing.css('span.a-price-fraction').text + " PLN"
            }
            items << item
            puts "#{item[:title]} | #{item[:price]}"
            puts ""
        end
        # when we're not on the last page, the "Dalej" button is enabled and it's an anchor tag which can be accessed like below
        next_page_element = parsed_page.css('a.s-pagination-item.s-pagination-next.s-pagination-button.s-pagination-separator')
        break if next_page_element.count == 0
        url = "https://www.amazon.pl" + next_page_element[0].attributes["href"].value
    end
    # byebug
end

# whole_book_div_class = "s-card-container.s-overflow-hidden.s-expand-height.s-include-content-margin.s-latency-cf-section.s-card-border"
# book_title_span_class = "a-size-base-plus.a-color-base.a-text-normal"
# book_price_whole_span_class = "a-price-whole"
# book_price_fraction_span_class = "a-price-fraction"

# "Dalej" button disabled -> span class="s-pagination-item s-pagination-next s-pagination-disabled "
# "Dalej" button enabled -> anchor class="s-pagination-item s-pagination-next s-pagination-button s-pagination-separator"

crawler